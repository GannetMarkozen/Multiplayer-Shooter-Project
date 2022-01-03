// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GASAbilitySystemComponent.h"

#include "GameplayCueManager.h"
#include "Character/ShooterCharacter.h"
#include "GAS/GASGameplayAbility.h"
#include "GameplayAbilities/Public/GameplayCueSet.h"
#include "GameplayAbilities/Public/GameplayCueNotify_Static.h"



bool UGASAbilitySystemComponent::BatchRPCTryActivateAbility(const FGameplayAbilitySpecHandle& AbilityHandle, const bool bEndAbilityImmediately)
{
	bool bAbilityActivated = false;
	if(AbilityHandle.IsValid())
	{
		FScopedServerAbilityRPCBatcher AbilityRPCBatcher(this, AbilityHandle);
		bAbilityActivated = TryActivateAbility(AbilityHandle, true);

		if(bEndAbilityImmediately)
		{
			if(const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilityHandle))
			{
				CastChecked<UGASGameplayAbility>(AbilitySpec->GetPrimaryInstance())->ExternalEndAbility();
			}
		}
	}
	
	return bAbilityActivated;
}

TArray<FGameplayAbilitySpecHandle> UGASAbilitySystemComponent::GetAbilitiesByClass(const TSubclassOf<UGASGameplayAbility>& Class) const
{
	if(!Class) return {};
	TArray<FGameplayAbilitySpecHandle> Handles;
	for(const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if(Spec.Ability->GetClass() == Class)
		{
			Handles.Add(Spec.Handle);
		}
	}
	return Handles;
}


void UGASAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters, const bool RPCLocalIfNecessary)
{
	if(RPCLocalIfNecessary) Client_ExecuteGameplayCueLocal(GameplayCueTag, GameplayCueParameters);
	else Client_ExecuteGameplayCueLocal_Implementation(GameplayCueTag, GameplayCueParameters);
}

void UGASAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::OnActive, GameplayCueParameters);
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::WhileActive, GameplayCueParameters);
}

void UGASAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Removed, GameplayCueParameters);
}

void UGASAbilitySystemComponent::AddLooseGameplayTagsForDuration(const FGameplayTagContainer& Tags, const float Duration, const int32 Count)
{
	if(Tags.IsEmpty()) return;
	PRINT(TEXT("Added %s"), *Tags.ToString());
	AddLooseGameplayTags(Tags, Count);
	FTimerHandle RemoveHandle;
	TimerHandles.Add(RemoveHandle);
	const auto& RemoveTags = [this, Tags, Count, &RemoveHandle]()->void
	{
		if(!IsValid(this)) return;
		RemoveLooseGameplayTags(Tags, Count);
		TimerHandles.Remove(RemoveHandle);
		PRINT(TEXT("Removed %s"), *Tags.ToString());
	};
	
	GetWorld()->GetTimerManager().SetTimer(RemoveHandle, RemoveTags, Duration, false);
}

void UGASAbilitySystemComponent::AbilityLocalInputPressed(int32 InputID)
{
	if(IsGenericConfirmInputBound(InputID))
	{
		LocalInputConfirm();
		return;
	}
	else if(IsGenericCancelInputBound(InputID))
	{
		LocalInputCancel();
		return;
	}
	
	ABILITYLIST_SCOPE_LOCK();
	for(FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if(Spec.InputID == InputID)
		{
			if(Spec.Ability)
			{
				Spec.InputPressed = true;
				
				if(Spec.IsActive())
				{
					if(Spec.Ability->bReplicateInputDirectly && !IsOwnerActorAuthoritative())
					{
						ServerSetInputPressed(Spec.Handle);
					}
                    
					AbilitySpecInputPressed(Spec);
                    
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
				}
				else
				{
					if(CastChecked<UGASGameplayAbility>(Spec.Ability)->bActivateOnPressed)
					{
						TryActivateAbility(Spec.Handle);
					}
				}
			}
		}
	}
}

void UGASAbilitySystemComponent::ClientActivateAbilityFailed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, int16 PredictionKey)
{
	Super::ClientActivateAbilityFailed_Implementation(AbilityToActivate, PredictionKey);

	if(const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate))
		if(UGASGameplayAbility* Ability = CastChecked<UGASGameplayAbility>(Spec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced ? Spec->Ability : Spec->GetPrimaryInstance()))
			CallClient_PredictionFailed(Ability, this);
}

// friend func
void CallClient_PredictionFailed(UGASGameplayAbility* Ability, UGASAbilitySystemComponent* ASC)
{
	if(Ability) Ability->Client_PredictionFailed((FGameplayAbilityActorInfoExtended&)*ASC->AbilityActorInfo.Get());
}

void UGASAbilitySystemComponent::ClientActivateAbilitySucceed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey)
{
	Super::ClientActivateAbilitySucceed_Implementation(AbilityToActivate, PredictionKey);

	if(const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate))
		if(UGASGameplayAbility* Ability = CastChecked<UGASGameplayAbility>(Spec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced ? Spec->Ability : Spec->GetPrimaryInstance()))
			CallClient_PredictionSucceeded(Ability, this);
}

// friend func
void CallClient_PredictionSucceeded(UGASGameplayAbility* Ability, UGASAbilitySystemComponent* ASC)
{
	if(Ability) Ability->Client_PredictionSucceeded((FGameplayAbilityActorInfoExtended&)*ASC->AbilityActorInfo.Get());
}

void UGASAbilitySystemComponent::ClientActivateAbilitySucceedWithEventData_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey, FGameplayEventData TriggerEventData)
{
	Super::ClientActivateAbilitySucceedWithEventData_Implementation(AbilityToActivate, PredictionKey, TriggerEventData);

	if(const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate))
		if(UGASGameplayAbility* Ability = CastChecked<UGASGameplayAbility>(Spec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced ? Spec->Ability : Spec->GetPrimaryInstance()))
			CallClient_PredictionSucceededWithEventData(Ability, TriggerEventData, this);
}

// friend func
void CallClient_PredictionSucceededWithEventData(UGASGameplayAbility* Ability, const FGameplayEventData& EventData, UGASAbilitySystemComponent* ASC)
{
	if(Ability) Ability->Client_PredictionSucceededWithEventData(EventData, (FGameplayAbilityActorInfoExtended&)*ASC->AbilityActorInfo.Get());
}




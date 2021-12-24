// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GASAbilitySystemComponent.h"

#include "Character/ShooterCharacter.h"
#include "GAS/GASGameplayAbility.h"



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
		if(UGASGameplayAbility* Ability = Cast<UGASGameplayAbility>(Spec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced ? Spec->Ability : Spec->GetPrimaryInstance()))
			CallClient_PredictionFailed(Ability);
}

// friend func
void CallClient_PredictionFailed(UGASGameplayAbility* Ability)
{
	if(Ability) Ability->Client_PredictionFailed();
}

void UGASAbilitySystemComponent::ClientActivateAbilitySucceed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey)
{
	Super::ClientActivateAbilitySucceed_Implementation(AbilityToActivate, PredictionKey);

	if(const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate))
		if(UGASGameplayAbility* Ability = Cast<UGASGameplayAbility>(Spec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced ? Spec->Ability : Spec->GetPrimaryInstance()))
			CallClient_PredictionSucceeded(Ability);
}

// friend func
void CallClient_PredictionSucceeded(UGASGameplayAbility* Ability)
{
	if(Ability) Ability->Client_PredictionSucceeded();
}

void UGASAbilitySystemComponent::ClientActivateAbilitySucceedWithEventData_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey, FGameplayEventData TriggerEventData)
{
	Super::ClientActivateAbilitySucceedWithEventData_Implementation(AbilityToActivate, PredictionKey, TriggerEventData);

	if(const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate))
		if(UGASGameplayAbility* Ability = Cast<UGASGameplayAbility>(Spec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced ? Spec->Ability : Spec->GetPrimaryInstance()))
			CallClient_PredictionSucceededWithEventData(Ability, TriggerEventData);
}

// friend func
void CallClient_PredictionSucceededWithEventData(UGASGameplayAbility* Ability, const FGameplayEventData& EventData)
{
	if(Ability) Ability->Client_PredictionSucceededWithEventData(EventData);
}



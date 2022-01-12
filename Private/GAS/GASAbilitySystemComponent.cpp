// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GASAbilitySystemComponent.h"


#include "GameplayCueManager.h"
#include "GameplayEffectExtension.h"
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

void UGASAbilitySystemComponent::BindAttributeChanged(UObject* Object, const FName FuncName, const FGameplayAttribute& Attribute)
{
	if(!Object || !FuncName.IsValid() || !Attribute.IsValid() || !HasAttributeSetForAttribute(Attribute)) return;

	if(UFunction* Func = Object->GetClass()->FindFunctionByName(FuncName))
	{
		const auto& Callback = [=](const FOnAttributeChangeData& Data)->void
		{
			if(!IsValid(Object)) return;
			FGameplayEffectSpecHandle Handle;
			if(Data.GEModData) Handle.Data = MakeShared<FGameplayEffectSpec>(Data.GEModData->EffectSpec);
			AttrChParams Params(Data.NewValue, Data.OldValue, Handle);
			Object->ProcessEvent(Func, &Params);
		};
	
		const FDelegateHandle& DelegateHandle = GetGameplayAttributeValueChangeDelegate(Attribute).AddLambda(Callback);
		DelegateHandles.Add(FObjAttrPair(Object, Attribute), DelegateHandle);
	}
}

void UGASAbilitySystemComponent::UnbindAttributeChanged(UObject* Object, const FGameplayAttribute& Attribute)
{
	if(!Object || !Attribute.IsValid()) return;
	
	if(const FDelegateHandle* Handle = DelegateHandles.Find(FObjAttrPair(Object, Attribute)))
	{
		GetGameplayAttributeValueChangeDelegate(Attribute).Remove(*Handle);
		DelegateHandles.Remove(FObjAttrPair(Object, Attribute));
	}
	else UE_LOG(LogTemp, Error, TEXT("Could not find %s %s set delegate pair"), *Object->GetName(), *Attribute.GetName());
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


void UGASAbilitySystemComponent::AddLooseGameplayTagForDuration(const FGameplayTag& Tag, const float Duration, const int32 Count, UObject* OptionalCallbackObject, const FName& OptionalCallbackFuncName)
{
	if(!Tag.IsValid()) return;
	AddLooseGameplayTag(Tag, Count);
	const TSharedPtr<FTimerHandle>& RemoveHandle = MakeShared<FTimerHandle>();
	TimerHandles.Add(Tag, RemoveHandle);
	const auto& RemoveTagFunc = [=]()->void
	{
		if(!IsValid(this)) return;
		RemoveLooseGameplayTag(Tag, Count);
		TimerHandles.Remove(Tag);
		if(IsValid(OptionalCallbackObject) && OptionalCallbackFuncName.IsValid())
		{
			if(UFunction* Func = OptionalCallbackObject->GetClass()->FindFunctionByName(OptionalCallbackFuncName))
			{
				FASCTagPair Params(this, Tag);
				OptionalCallbackObject->ProcessEvent(Func, &Params);
			}
		}
	};
	
	GetWorld()->GetTimerManager().SetTimer(*RemoveHandle, RemoveTagFunc, Duration, false);
}

void UGASAbilitySystemComponent::AddLooseGameplayTagForDurationSingle(const FGameplayTag& Tag, const float Duration, UObject* OptionalCallbackObject, const FName& OptionalCallbackFuncName)
{
	if(!Tag.IsValid()) return;
	if(FTimerHandle** HandlePtr = (FTimerHandle**)TimerHandles.Find(Tag))
	{// Clear timer and reset with new duration if it exceeds current remaining duration
		FTimerHandle& Handle = **HandlePtr;
		if(GetWorld()->GetTimerManager().GetTimerRemaining(Handle) < Duration)
		{
			const auto& RemoveTagFunc = [=]()->void
			{
				if(!IsValid(this)) return;
				RemoveLooseGameplayTag(Tag, 1);
				TimerHandles.Remove(Tag);
				if(IsValid(OptionalCallbackObject) && OptionalCallbackFuncName.IsValid())
				{
					if(UFunction* Func = OptionalCallbackObject->GetClass()->FindFunctionByName(OptionalCallbackFuncName))
					{
						FASCTagPair Params(this, Tag);
						OptionalCallbackObject->ProcessEvent(Func, &Params);
					}
				}
			};
			GetWorld()->GetTimerManager().ClearTimer(Handle);
			GetWorld()->GetTimerManager().SetTimer(Handle, RemoveTagFunc, Duration, false);
		}
	}
	else
	{// If no current active duration tags, create one
		AddLooseGameplayTagForDuration(Tag, Duration, 1, OptionalCallbackObject, OptionalCallbackFuncName);
	}
}

void UGASAbilitySystemComponent::AddLooseGameplayTagForDuration_Static(const FGameplayTag& Tag, const float Duration, const int32 Count, const TDelegate<void(UGASAbilitySystemComponent*, const FGameplayTag&)>* OptionalDelegate)
{
	if(!Tag.IsValid()) return;
	AddLooseGameplayTag(Tag, Count);
	const TSharedPtr<FTimerHandle>& RemoveHandle = MakeShared<FTimerHandle>();
	TimerHandles.Add(Tag, RemoveHandle);
	const TDelegate<void(UGASAbilitySystemComponent*, const FGameplayTag&)>& Callback = OptionalDelegate ? *OptionalDelegate : TDelegate<void(UGASAbilitySystemComponent*, const FGameplayTag&)>();
	const auto& RemoveTagFunc = [=]()->void
	{
		if(!IsValid(this)) return;
		RemoveLooseGameplayTag(Tag, Count);
		TimerHandles.Remove(Tag);
		if(Callback.IsBound())
			Callback.Execute(this, Tag);
	};
	
	GetWorld()->GetTimerManager().SetTimer(*RemoveHandle, RemoveTagFunc, Duration, false);
}

void UGASAbilitySystemComponent::AddLooseGameplayTagForDurationSingle_Static(const FGameplayTag& Tag, const float Duration, const TDelegate<void(UGASAbilitySystemComponent*, const FGameplayTag&)>* OptionalDelegate)
{
	if(!Tag.IsValid()) return;
	if(FTimerHandle** HandlePtr = (FTimerHandle**)TimerHandles.Find(Tag))
	{// Clear timer and reset with new duration if it exceeds current remaining duration
		FTimerHandle& Handle = **HandlePtr;
		if(GetWorld()->GetTimerManager().GetTimerRemaining(Handle) < Duration)
		{
			const TDelegate<void(UGASAbilitySystemComponent*, const FGameplayTag&)>& Callback = OptionalDelegate ? *OptionalDelegate : TDelegate<void(UGASAbilitySystemComponent*, const FGameplayTag&)>();
			const auto& RemoveTagFunc = [=]()->void
			{
				if(!IsValid(this)) return;
				RemoveLooseGameplayTag(Tag, 1);
				TimerHandles.Remove(Tag);
				if(Callback.IsBound())
					Callback.Execute(this, Tag);
			};
			GetWorld()->GetTimerManager().ClearTimer(Handle);
			GetWorld()->GetTimerManager().SetTimer(Handle, RemoveTagFunc, Duration, false);
		}
	}
	else
	{// If no current active duration tags, create one
		AddLooseGameplayTagForDuration_Static(Tag, Duration, 1, OptionalDelegate);
	}
}

bool UGASAbilitySystemComponent::CancelLooseGameplayTagDuration(const FGameplayTag& Tag)
{
	if(!Tag.IsValid()) return false;
	if(FTimerHandle** HandlePtr = (FTimerHandle**)TimerHandles.Find(Tag))
	{
		GetWorld()->GetTimerManager().ClearTimer(**HandlePtr);
		TimerHandles.Remove(Tag);
		RemoveLooseGameplayTag(Tag);
		return true;
	}
	return false;
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

FGameplayAttributeData* UGASAbilitySystemComponent::FindAttributeData(const FGameplayAttribute& Attribute) const
{
	if(Attribute.IsValid())
		for(UAttributeSet* Set : GetSpawnedAttributes())
			if(Set->GetClass()->IsChildOf(Attribute.GetAttributeSetClass()))
				return Attribute.GetUProperty()->ContainerPtrToValuePtr<FGameplayAttributeData>(Set);
			
	return nullptr;
}



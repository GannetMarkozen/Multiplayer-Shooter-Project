// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExtendedTypes.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "GASGameplayAbility.generated.h"

// Use this macro in non-instanced abilities to get the FGameplayAbilityInfoExtended* static_cast from the ActorInfo param

#define ACTORINFO static_cast<const FGameplayAbilityActorInfoExtended*>(ActorInfo)
#define CHARACTER static_cast<const FGameplayAbilityActorInfoExtended*>(ActorInfo)->Character.Get()
#define GET_ASC static_cast<const FGameplayAbilityActorInfoExtended*>(ActorInfo)->ASC.Get()
#define CHARACTERMOVEMENT static_cast<const FGameplayAbilityActorInfoExtended*>(ActorInfo)->Character.Get()->GetCharacterMovement()
#define INVENTORY static_cast<const FGameplayAbilityActorInfoExtended*>(ActorInfo)->Inventory.Get()




DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPredictionDelegate, const FGameplayAbilityActorInfoExtended&, ActorInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPredictionDelegateWithEventData, const FGameplayEventData&, EventData, const FGameplayAbilityActorInfoExtended&, ActorInfo);
/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UGASGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGASGameplayAbility();
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Ability")
	FORCEINLINE void ExternalEndAbility() { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	EAbilityInput Input = EAbilityInput::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "Input != EAbilityInput::None"), Category = "Input")
	bool bActivateOnPressed = true;

	friend void CallClient_PredictionFailed(class UGASGameplayAbility* Ability, class UGASAbilitySystemComponent* ASC);
	friend void CallClient_PredictionSucceeded(class UGASGameplayAbility* Ability, class UGASAbilitySystemComponent* ASC);
	friend void CallClient_PredictionSucceededWithEventData(class UGASGameplayAbility* Ability, const FGameplayEventData& EventData, class UGASAbilitySystemComponent* ASC);

	UPROPERTY(BlueprintAssignable)
	FPredictionDelegate OnSuccessfulPrediction;

	UPROPERTY(BlueprintAssignable)
	FPredictionDelegate OnFailedPrediction;

	// Called on successful predictions with EventData
	UPROPERTY(BlueprintAssignable)
	FPredictionDelegateWithEventData OnSuccessfulPredictionWithEventData;

	// static casts CurrentActorInfo to const FGameplayAbilityActorInfoExtended*
	const FGameplayAbilityActorInfoExtended* GetActorInfoExtended() const
	{
		return static_cast<const FGameplayAbilityActorInfoExtended*>(CurrentActorInfo);
	}

	UFUNCTION(BlueprintPure, Category = "ActorInfo")
	FORCEINLINE class AShooterCharacter* GetCharacter() const
	{
		return GetActorInfoExtended()->Character.Get();
	}

	UFUNCTION(BlueprintPure, Category = "ActorInfo")
	FORCEINLINE class UCharacterInventoryComponent* GetInventory() const
	{
		return GetActorInfoExtended()->Inventory.Get();
	}

	UFUNCTION(BlueprintPure, Category = "ActorInfo")
	FORCEINLINE class UGASAbilitySystemComponent* GetASC() const
	{
		return GetActorInfoExtended()->ASC.Get();
	}

	UFUNCTION(BlueprintPure, Category = "ActorInfo")
	FORCEINLINE class UCharacterMovementComponent* GetCharacterMovement() const;
	
protected:
	virtual FORCEINLINE void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override
	{
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
		if(ActorInfo)
			ActivateAbilityExtended(Handle, (FGameplayAbilityActorInfoExtended&)*ActorInfo, ActivationInfo, TriggerEventData ? *TriggerEventData : FGameplayEventData());
	}

	virtual FORCEINLINE void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override
	{
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		if(ActorInfo)
			EndAbilityExtended(Handle, (FGameplayAbilityActorInfoExtended&)*ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|Ability")
	void ActivateAbilityExtended(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActorInfoExtended& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayEventData& TriggerEventData);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "GAS|Ability")
	void EndAbilityExtended(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActorInfoExtended& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo, bool bReplicateEndAbility = false, bool bWasCancelled = false);
	
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "On Give Ability"), Category = "GAS|Ability")
	void K2_OnGiveAbility(const FGameplayAbilityActorInfoExtended& ActorInfo, const FGameplayAbilitySpec& AbilitySpec);

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "On Remove Ability"), Category = "GAS|Ability")
	void K2_OnRemoveAbility(const FGameplayAbilityActorInfoExtended& ActorInfo, const FGameplayAbilitySpec& AbilitySpec);
	
	// Client-side failed ability prediction, Always call parent function
	UFUNCTION(BlueprintNativeEvent, Category = "GAS|Ability")
	void Client_PredictionFailed(const FGameplayAbilityActorInfoExtended& ActorInfo);
	virtual FORCEINLINE void Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
	{
		OnFailedPrediction.Broadcast(ActorInfo);
	}

	// Client-side successful ability prediction, Always call parent function
	UFUNCTION(BlueprintNativeEvent, Category = "GAS|Ability")
	void Client_PredictionSucceeded(const FGameplayAbilityActorInfoExtended& ActorInfo);
	virtual FORCEINLINE void Client_PredictionSucceeded_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
	{
		OnSuccessfulPrediction.Broadcast(ActorInfo);
	}

	// Client-side successful ability prediction with event data, Always call parent function
	UFUNCTION(BlueprintNativeEvent, Category = "GAS|Ability")
	void Client_PredictionSucceededWithEventData(const FGameplayEventData& EventData, const FGameplayAbilityActorInfoExtended& ActorInfo);
	virtual FORCEINLINE void Client_PredictionSucceededWithEventData_Implementation(const FGameplayEventData& EventData, const FGameplayAbilityActorInfoExtended& ActorInfo)
	{
		OnSuccessfulPredictionWithEventData.Broadcast(EventData, ActorInfo);
	}

private:
	UFUNCTION(BlueprintPure, Meta = (DisplayName = "Get Current Actor Info Extended", AllowPrivateAccess = "true"), Category = "ActorInfo")
	const FORCEINLINE FGameplayAbilityActorInfoExtended& BP_GetCurrentActorInfoExtended() const
	{
		return *GetActorInfoExtended();
	}
protected:
	// Abilities added when this ability is given and removed when this ability is removes
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Ability")
	TArray<TSubclassOf<class UGASGameplayAbility>> OwnedAbilities;
	
	// Cooldown overrides
	/*
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldown")
	FScalableFloat CooldownDuration;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldown")
	FGameplayTagContainer CooldownTags;

	// Temp only
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;*/
};

UCLASS()
class MULTIPLAYERSHOOTER_API UCooldownEffect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UCooldownEffect()
	{
		DurationPolicy = EGameplayEffectDurationType::HasDuration;
		FSetByCallerFloat SetByCaller;
		SetByCaller.DataTag = TAG("Data.Cooldown");
		DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
	}
};
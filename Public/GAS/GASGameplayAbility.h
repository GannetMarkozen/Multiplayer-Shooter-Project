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
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "On Give Ability"), Category = "GAS|Ability")
	void K2_OnGiveAbility(const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilitySpec& AbilitySpec);
	
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

	constexpr static int AbilityIdentifier = 1;

private:
	UFUNCTION(BlueprintPure, Meta = (DisplayName = "Get Current Actor Info Extended", AllowPrivateAccess = "true"), Category = "ActorInfo")
	const FORCEINLINE FGameplayAbilityActorInfoExtended& BP_GetCurrentActorInfoExtended() const
	{
		return *GetActorInfoExtended();
	}
};
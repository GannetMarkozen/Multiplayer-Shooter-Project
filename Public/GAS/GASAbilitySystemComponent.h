// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GASGameplayAbility.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "MultiplayerShooter/MultiplayerShooter.h"

#include "GASAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UGASAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "GAS")
	bool BatchRPCTryActivateAbility(const FGameplayAbilitySpecHandle& AbilityHandle, const bool bEndAbilityImmediately);
	
protected:
	virtual void AbilityLocalInputPressed(int32 InputID) override;
	virtual void ClientActivateAbilityFailed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, int16 PredictionKey) override;
	virtual void ClientActivateAbilitySucceed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey) override;
	virtual void ClientActivateAbilitySucceedWithEventData_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey, FGameplayEventData TriggerEventData) override;
	
	virtual FORCEINLINE bool ShouldDoServerAbilityRPCBatch() const override { return true; }
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Loose Gameplay Tags"), Category = "GAS")
	FORCEINLINE void BP_AddLooseGameplayTags(const FGameplayTagContainer Tags, const int32 Count = 1) { AddLooseGameplayTags(Tags, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Loose Gameplay Tags"), Category = "GAS")
	FORCEINLINE void BP_RemoveLooseGameplayTags(const FGameplayTagContainer Tags, const int32 Count = 1) { RemoveLooseGameplayTags(Tags, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Loose Gameplay Tag"), Category = "GAS")
	FORCEINLINE void BP_AddLooseGameplayTag(const FGameplayTag Tag, const int32 Count = 1) { AddLooseGameplayTag(Tag, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Loose Gameplay Tag"), Category = "GAS")
	FORCEINLINE void BP_RemoveLooseGameplayTag(const FGameplayTag Tag, const int32 Count = 1) { RemoveLooseGameplayTag(Tag, Count); }
};

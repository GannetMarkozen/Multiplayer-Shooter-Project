// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
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

	UFUNCTION(BlueprintPure, Meta = (AutoCreateRefTerm = "Class"), Category = "GAS")
	TArray<FGameplayAbilitySpecHandle> GetAbilitiesByClass(const TSubclassOf<class UGASGameplayAbility>& Class) const;

	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"), Category = "GameplayCue")
	FORCEINLINE void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters, const bool RPCLocalIfNecessary = true);

	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"), Category = "GameplayCue")
	FORCEINLINE void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"), Category = "GameplayCue")
	FORCEINLINE void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Meta = (DisplayName = "Make Effect Context Extended"), Category = "GAS")
	FORCEINLINE FGameplayEffectContextHandle K2_MakeEffectContextExtended(class AActor* OptionalTargetActor, const FGameplayAbilityTargetDataHandle OptionalTargetData) const
	{
		return MakeEffectContextExtended(OptionalTargetActor, OptionalTargetData);
	}

	FORCEINLINE FGameplayEffectContextHandle MakeEffectContextExtended(class AActor* OptionalTarget = nullptr, const FGameplayAbilityTargetDataHandle& OptionalTargetDataHandle = FGameplayAbilityTargetDataHandle()) const
	{
		const FGameplayEffectContextHandle& Handle = MakeEffectContext();
		if(!OptionalTargetDataHandle.Data.IsEmpty()) ((FGameplayEffectContextExtended*)Handle.Get())->AddTargetData(OptionalTargetDataHandle);
		if(OptionalTarget) ((FGameplayEffectContextExtended*)Handle.Get())->SetTarget(OptionalTarget);
		return Handle;
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Loose Gameplay Tags"), Category = "GAS")
	FORCEINLINE void BP_AddLooseGameplayTags(const FGameplayTagContainer Tags, const int32 Count = 1) { AddLooseGameplayTags(Tags, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Loose Gameplay Tags"), Category = "GAS")
	FORCEINLINE void BP_RemoveLooseGameplayTags(const FGameplayTagContainer Tags, const int32 Count = 1) { RemoveLooseGameplayTags(Tags, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Loose Gameplay Tag"), Category = "GAS")
	FORCEINLINE void BP_AddLooseGameplayTag(const FGameplayTag Tag, const int32 Count = 1) { AddLooseGameplayTag(Tag, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Loose Gameplay Tag"), Category = "GAS")
	FORCEINLINE void BP_RemoveLooseGameplayTag(const FGameplayTag Tag, const int32 Count = 1) { RemoveLooseGameplayTag(Tag, Count); }

	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "Tags"), Category = "GAS")
	void AddLooseGameplayTagsForDuration(const FGameplayTagContainer& Tags, const float Duration, const int32 Count = 1);
	
protected:
	virtual void AbilityLocalInputPressed(int32 InputID) override;
	virtual void ClientActivateAbilityFailed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, int16 PredictionKey) override;
	virtual void ClientActivateAbilitySucceed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey) override;
	virtual void ClientActivateAbilitySucceedWithEventData_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey, FGameplayEventData TriggerEventData) override;
	virtual FORCEINLINE bool ShouldDoServerAbilityRPCBatch() const override { return true; }
	
	UFUNCTION(Client, Reliable)
	void Client_ExecuteGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& Params);
	FORCEINLINE void Client_ExecuteGameplayCueLocal_Implementation(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& Params)
	{
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Executed, Params);
	}

	TArray<FTimerHandle> TimerHandles;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "EquipWeaponAbility.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UEquipWeaponAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UEquipWeaponAbility();
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void Client_PredictionFailed_Implementation() override;
	
	virtual void Equip(const int32 NewIndex, bool bCancelled = false);

	UPROPERTY(BlueprintReadWrite, Category = "Weapons")
	TArray<FGameplayAbilitySpecHandle> EquippedWeaponSpecHandles;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	static bool EquipWeaponEvent(class UAbilitySystemComponent* TargetASC, const int32 Index);
};

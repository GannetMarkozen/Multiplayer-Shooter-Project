// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "EquipAbility.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UEquipAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UEquipAbility();
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void Client_PredictionSucceeded_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo) override;
	virtual void Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAS|Ability")
	void Equip(class AWeapon* NewWeapon, class AWeapon* OldWeapon, const FGameplayAbilityActorInfoExtended& ActorInfo);
	void Equip_Implementation(class AWeapon* NewWeapon, class AWeapon* OldWeapon, const FGameplayAbilityActorInfoExtended& ActorInfo);

public:	
	UFUNCTION(BlueprintCallable, Category = "GAS|Ability")
	static void EquipWeapon(class UAbilitySystemComponent* ASC, int32 Index);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "DropItemAbility.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UDropItemAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UDropItemAbility();
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	float DropVelocity = 20000.f;
};

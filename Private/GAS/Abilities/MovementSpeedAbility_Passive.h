// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "MovementSpeedAbility_Passive.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UMovementSpeedAbility_Passive : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UMovementSpeedAbility_Passive();

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void MovementSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void MovementSpeedChanged_Implementation(const FOnAttributeChangeData& Data);
};

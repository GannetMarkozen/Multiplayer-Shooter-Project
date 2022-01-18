// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Components/TimelineComponent.h"
#include "GAS/GASGameplayAbility.h"
#include "ADSAbility.generated.h"

/**
 * Aim down sights ability.
 * Set socket on skelmesh weapon for
 * "sights"
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UADSAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UADSAbility();
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability|ADS")
	void SecondaryFireReleased(float Time);
	virtual void SecondaryFireReleased_Implementation(float Time);

	UFUNCTION(BlueprintNativeEvent, Category = "Ability|ADS")
	void TimelineProgress(float DeltaTime, float Progress, float Value);
	virtual void TimelineProgress_Implementation(float DeltaTime, float Progress, float Value);

	UFUNCTION(BlueprintNativeEvent, Category = "Ability|ADS")
	void TimelineEnd(float DeltaTime, float Progress, float Value);
	virtual void TimelineEnd_Implementation(float DeltaTime, float Progress, float Value);

	UPROPERTY(BlueprintReadWrite, Category = "State")
	class UAbilityTask_PlayTimeline* Task;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	FGameplayTag AimingTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	class UCurveFloat* AimingCurveFloat;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Configurations")
	float AimingProgress = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	float AimingPlayRate = 1.f;

	// Is init on activate ability
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	FTransform TargetTransform;
};

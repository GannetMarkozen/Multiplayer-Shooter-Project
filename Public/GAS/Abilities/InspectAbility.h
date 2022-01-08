// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "InspectAbility.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UInspectAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UInspectAbility();
protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	float InspectFrequency = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	float InspectDistance = 400.f;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	TEnumAsByte<ECollisionChannel> InspectCollisionChannel = ECC_Projectile;

	FTimerHandle TimerHandle;
	
	// Exposed for the interact ability to modify
	UPROPERTY(BlueprintReadWrite, Category = "Inspect Ability")
	class AActor* InspectActor;

	friend FORCEINLINE class AActor* GetInspectActor(const class UInspectAbility* InspectAbility);
};

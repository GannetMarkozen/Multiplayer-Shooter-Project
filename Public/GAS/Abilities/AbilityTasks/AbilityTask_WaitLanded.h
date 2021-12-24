// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilities/Public/Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitLanded.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanded, class ACharacter*, OwningCharacter);
/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UAbilityTask_WaitLanded : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_WaitLanded();

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"), Category = "Ability|Tasks")
	static class UAbilityTask_WaitLanded* WaitLanded(class UGameplayAbility* OwningAbility, class ACharacter* OwningCharacter);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Ability|Tasks|Delegates")
	FOnLanded LandedDelegate;
	
protected:
	UPROPERTY()
	class ACharacter* Character;

	UFUNCTION()
	void OnLandedCallback(const FHitResult& Hit);
};

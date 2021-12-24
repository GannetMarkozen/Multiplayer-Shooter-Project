// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "MultiplayerShooter/MultiplayerShooter.h"

#include "AbilityTask_WaitMontageCompleted.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMontageCompleted);
/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UAbilityTask_WaitMontageCompleted : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_WaitMontageCompleted();
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PlayMontageCompleted", DefaultToSelf = "OwningAbility", HidePin = "OwningAbility"), Category = "Ability|Tasks")
	static class UAbilityTask_WaitMontageCompleted* WaitMontageCompleted(class UGASGameplayAbility* OwningAbility, class UAnimInstance* AnimInstance, class UAnimMontage* Montage,
		const float PlayRate = 1.f, const float StartTimeSeconds = 0.f, const bool bStopOnEndAbility = true, const FName& TaskInstanceName = "WaitMontageCompleted");

	virtual void Activate() override;

	// Delegate called on montage completed
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FMontageCompleted MontageCompletedDelegate;
protected:
	UFUNCTION()
	void MontageCompleted(class UAnimMontage* AnimMontage, bool bInterrupted);

	UFUNCTION()
	void EndMontage();
	
	UPROPERTY()
	TObjectPtr<class UAnimInstance> AnimInstance;

	UPROPERTY()
	TObjectPtr<class UAnimMontage> Montage;

	float PlayRate;
	float StartTimeSeconds;
	bool bStopOnEndAbility;

	FOnMontageEnded MontageEndedDelegate;
};

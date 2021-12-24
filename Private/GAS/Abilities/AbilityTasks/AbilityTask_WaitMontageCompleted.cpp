// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/AbilityTasks/AbilityTask_WaitMontageCompleted.h"

#include "AbilitySystemComponent.h"
#include "GAS/GASGameplayAbility.h"

UAbilityTask_WaitMontageCompleted::UAbilityTask_WaitMontageCompleted()
{
	
}


UAbilityTask_WaitMontageCompleted* UAbilityTask_WaitMontageCompleted::WaitMontageCompleted(UGASGameplayAbility* OwningAbility, UAnimInstance* AnimInstance, UAnimMontage* Montage,
	const float PlayRate, const float StartTimeSeconds, const bool bStopOnEndAbility, const FName& TaskInstanceName)
{
	UAbilityTask_WaitMontageCompleted* Task = NewAbilityTask<UAbilityTask_WaitMontageCompleted>(OwningAbility, TaskInstanceName);
	Task->AnimInstance = AnimInstance;
	Task->Montage = Montage;
	Task->PlayRate = PlayRate;
	Task->StartTimeSeconds = StartTimeSeconds;
	Task->bStopOnEndAbility = bStopOnEndAbility;
	return Task;
}

void UAbilityTask_WaitMontageCompleted::Activate()
{
	Super::Activate();

	if(!AnimInstance || !Montage)
	{
		EndTask();
		return;
	}

	if(Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted && !Ability->GetCurrentActorInfo()->IsNetAuthority())
		CastChecked<UGASGameplayAbility>(Ability)->OnFailedPrediction.AddDynamic(this, &UAbilityTask_WaitMontageCompleted::EndMontage);
	
	MontageEndedDelegate.BindUObject(this, &UAbilityTask_WaitMontageCompleted::MontageCompleted);
	AnimInstance->Montage_Play(Montage, PlayRate, EMontagePlayReturnType::MontageLength, StartTimeSeconds);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
}
/*
void UAbilityTask_WaitMontageCompleted::OnDestroy(bool bAbilityEnded)
{
	Super::OnDestroy(bAbilityEnded);

	if(bAbilityEnded)
	{
		AnimInstance->Montage_Stop(0.f, Montage);
	}
}*/

void UAbilityTask_WaitMontageCompleted::EndMontage()
{
	AnimInstance->Montage_Stop(0.f, Montage);

	MontageCompleted(Montage, true);
}


void UAbilityTask_WaitMontageCompleted::MontageCompleted(UAnimMontage* AnimMontage, bool bInterrupted)
{
	EndTask();
	
	MontageCompletedDelegate.Broadcast();
}



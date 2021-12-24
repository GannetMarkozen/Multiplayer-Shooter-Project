// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/AbilityTasks/AbilityTask_WaitLanded.h"

#include "GameFramework/Character.h"


UAbilityTask_WaitLanded::UAbilityTask_WaitLanded()
{
	
}

UAbilityTask_WaitLanded* UAbilityTask_WaitLanded::WaitLanded(UGameplayAbility* OwningAbility, ACharacter* OwningCharacter)
{
	UAbilityTask_WaitLanded* Task = NewAbilityTask<UAbilityTask_WaitLanded>(OwningAbility);
	Task->Character = OwningCharacter;
	return Task;
}

void UAbilityTask_WaitLanded::Activate()
{
	Super::Activate();

	if(Character)
	{
		Character->LandedDelegate.AddDynamic(this, &UAbilityTask_WaitLanded::OnLandedCallback);
	}
}

void UAbilityTask_WaitLanded::OnLandedCallback(const FHitResult& Hit)
{
	if(Character)
	{
		LandedDelegate.Broadcast(Character);
	}
	EndTask();
}




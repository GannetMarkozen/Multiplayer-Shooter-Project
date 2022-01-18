// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSAbility.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/CharacterInventoryComponent.h"
#include "Character/ShooterCharacter.h"
#include "GAS/Abilities/AbilityTasks/AbilityTask_PlayTimeline.h"


UADSAbility::UADSAbility()
{
	Input = EAbilityInput::SecondaryFire;

	AimingTag = TAG("WeaponState.Aiming");
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	//ActivationOwnedTags.AddTag(TAG("WeaponState.Aiming"));

	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Reloading"));
}

bool UADSAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && GetInventory()->GetCurrentWeapon() && GetInventory()->GetCurrentWeapon()->CanFire();
}

void UADSAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if(!CHARACTER->IsLocallyControlled())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	// Wait for input release
	if(UAbilityTask_WaitInputRelease* WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UADSAbility::SecondaryFireReleased);
		WaitInputReleaseTask->Activate();
	}
	else SecondaryFireReleased(0.f);

	Task = UAbilityTask_PlayTimeline::PlayTimeline(this, FName("Something"), AimingCurveFloat, 0.f, AimingPlayRate, ETimelineDirection::Forward, false);
	if(Task)
	{
		Task->TimelineProgress.AddDynamic(this, &UADSAbility::TimelineProgress);
		Task->TimelineEnd.AddDynamic(this, &UADSAbility::TimelineEnd);
		Task->Activate();
	}
	else {PRINT(TEXT("Failed to make task"));}
}

void UADSAbility::SecondaryFireReleased_Implementation(float Time)
{
	if(Task) Task->Reverse();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UADSAbility::TimelineProgress_Implementation(float DeltaTime, float Progress, float Value)
{
	PRINT(TEXT("TimelineProgress DeltaTime == %f, Progress == %f, Value == %f"), DeltaTime, Progress, Value);
}

void UADSAbility::TimelineEnd_Implementation(float DeltaTime, float Progress, float Value)
{
	PRINT(TEXT("TimelineEnd Value == %f"), Value);
}






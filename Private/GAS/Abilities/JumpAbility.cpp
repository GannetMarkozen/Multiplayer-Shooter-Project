// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/JumpAbility.h"

#include "Character/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemGlobals.h"
#include "Chaos/Collision/CollisionApplyType.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/Abilities/AbilityTasks/AbilityTask_WaitLanded.h"

UJumpAbility::UJumpAbility()
{
	Input = EAbilityInput::Jump;
	
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;

	AbilityTags.AddTag(TAG("Ability.Jump"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));
}

bool UJumpAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{// Only activate if not falling
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && CHARACTER->CanJump();
}

void UJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// Need to cast to character from actor info because this is playing on the CDO and thus the shooter character reference is based off of the character with 0th net connection
	if(HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) && CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CHARACTERMOVEMENT->DoJump(false);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}


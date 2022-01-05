// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GASGameplayAbility.h"
#include "Character/ShooterCharacter.h"
#include "GameplayAbilities/Public/GameplayCueManager.h"

void UGASGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	K2_OnGiveAbility((FGameplayAbilityActorInfoExtended&)*ActorInfo, Spec);
}

UCharacterMovementComponent* UGASGameplayAbility::GetCharacterMovement() const
{
	return GetActorInfoExtended()->Character.Get()->GetCharacterMovement();
}



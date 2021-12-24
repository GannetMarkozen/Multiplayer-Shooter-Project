// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GASGameplayAbility.h"
#include "Character/ShooterCharacter.h"

void UGASGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	/*
	if(InstancingPolicy != EGameplayAbilityInstancingPolicy::NonInstanced)
	if(ShooterCharacter = Cast<AShooterCharacter>(ActorInfo->AvatarActor))
	{
		ASC = ShooterCharacter->GetASC();
		Inventory = ShooterCharacter->GetCharacterInventory();
	}*/
}

UCharacterMovementComponent* UGASGameplayAbility::GetCharacterMovement() const
{
	return GetActorInfoExtended()->Character.Get()->GetCharacterMovement();
}

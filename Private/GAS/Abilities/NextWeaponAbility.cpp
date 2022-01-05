// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/NextWeaponAbility.h"

#include "AbilitySystemComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "GAS/Abilities/EquipAbility.h"
#include "GAS/GASAbilitySystemComponent.h"

UNextWeaponAbility::UNextWeaponAbility()
{
	Input = EAbilityInput::MWheelUp;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UNextWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
/*
	const int32 CurrentIndex = CastChecked<UEquipWeaponAbility>(ActorInfo->AbilitySystemComponent.Get()->FindAbilitySpecFromClass(UEquipWeaponAbility::StaticClass())->Ability)->GetCurrentIndex();
	const int32 NewIndex = IInventoryInterface::Execute_GetInventory(ActorInfo->AvatarActor.Get())->GetWeapons().IsValidIndex(CurrentIndex + 1) ? CurrentIndex + 1 : 0;
	UEquipWeaponAbility::EquipWeaponEvent(ActorInfo->AbilitySystemComponent.Get(), NewIndex);*/
	
	const int32 NewIndex = INVENTORY->GetWeapons().IsValidIndex(INVENTORY->GetCurrentIndex() + 1) ? INVENTORY->GetCurrentIndex() + 1 : 0;
	UEquipAbility::EquipWeapon(GET_ASC, NewIndex);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}


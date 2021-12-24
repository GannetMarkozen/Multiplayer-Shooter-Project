// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Weapons/Weapon.h"

#include "Character/ShooterCharacter.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/GASGameplayAbility.h"
#include "GAS/Effects/DamageEffect.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	DamageEffect = UDamageEffect::StaticClass();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, Ammo, COND_OwnerOnly, REPNOTIFY_OnChanged);
}

void AWeapon::OnObtained_Implementation(UInventoryComponent* Inventory)
{
	CurrentInventory = Inventory;
	if(CurrentOwner = Cast<AShooterCharacter>(CurrentInventory ? CurrentInventory->GetOwner() : nullptr))
	{
		CurrentCharacterInventory = CurrentOwner->GetCharacterInventory();
		CurrentASC = CurrentOwner->GetASC();
	}
}

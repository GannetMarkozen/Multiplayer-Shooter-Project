// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CharacterInventoryComponent.h"

#include "Character/ShooterCharacter.h"
#include "GAS/Abilities/EquipAbility.h"
#include "Net/UnrealNetwork.h"


UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	
}

void UCharacterInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}


void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = CastChecked<AShooterCharacter>(GetOwner());
	
	// Equips DefaultItemIndex from inventory. Only functions with the equip gameplay ability
	if(OwningCharacter->HasAuthority())
	{
		const auto& Equip = [this]()->void
		{
			if(!IsValid(this)) return;
			UEquipAbility::EquipWeapon(OwningCharacter->GetASC(), DefaultItemIndex);
		};
		
		GetWorld()->GetTimerManager().SetTimer(EquipDelayTimerHandle, Equip, 0.1f, false);
		EquipDelayTimerHandle.Invalidate();
	}
}

int32 UCharacterInventoryComponent::AddItems(const TArray<AWeapon*>& NewWeapons)
{
	const int32 Num = Super::AddItems(NewWeapons);
	
	if(Weapons.IsValidIndex(DefaultItemIndex) && Weapons[DefaultItemIndex] && !OwningCharacter->GetCurrentWeapon())
		UEquipAbility::EquipWeapon(OwningCharacter->GetASC(), DefaultItemIndex);

	return Num;
}

void UCharacterInventoryComponent::RemoveItem(const int32 Index)
{
	Super::RemoveItem(Index);

	if(Index <= CurrentIndex)
	{
		if(Weapons.IsValidIndex(CurrentIndex - 1))
		{
			UEquipAbility::EquipWeapon(OwningCharacter->GetASC(), DefaultItemIndex);
		}
		else
		{
			UEquipAbility::EquipWeapon(OwningCharacter->GetASC(), 0);
		}
	}
}



void UCharacterInventoryComponent::GiveAbilities(const AWeapon* Weapon)
{
	if(!Weapon) return;
	for(const TSubclassOf<UGASGameplayAbility>& AbilityClass : Weapon->GetWeaponAbilities())
	{
		const FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, static_cast<int32>(AbilityClass.GetDefaultObject()->Input), this);
		ActiveWeaponAbilities.Add(OwningCharacter->GetASC()->GiveAbility(AbilitySpec));
	}
}

void UCharacterInventoryComponent::RemoveAbilities()
{
	for(const FGameplayAbilitySpecHandle& Handle : ActiveWeaponAbilities)
	{
		OwningCharacter->GetASC()->CancelAbilityHandle(Handle);
		OwningCharacter->GetASC()->SetRemoveAbilityOnEnd(Handle);
	}
	ActiveWeaponAbilities.Empty();
}


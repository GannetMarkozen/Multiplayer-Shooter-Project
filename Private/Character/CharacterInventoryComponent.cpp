// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CharacterInventoryComponent.h"

#include "Character/ShooterCharacter.h"
#include "GAS/Abilities/EquipAbility.h"
#include "Net/UnrealNetwork.h"

// AWeapon friend func, only calls if weapon is valid
void CallOnEquipped(AWeapon* Weapon, UCharacterInventoryComponent* Inventory)
{
	if(Weapon) Weapon->OnEquipped(Inventory);
}
// AWeapon friend func, only calls if weapon is valid
void CallOnUnEquipped(AWeapon* Weapon, UCharacterInventoryComponent* Inventory)
{
	if(Weapon) Weapon->OnUnEquipped(Inventory);
}



void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningCharacter = CastChecked<AShooterCharacter>(GetOwner());
	if(OwningCharacter->HasAuthority())
	{
		const auto& EquipDefault = [this]()->void{ SetCurrentWeapon(0); };
		GetWorld()->GetTimerManager().SetTimerForNextTick(EquipDefault);
	}
}

void UCharacterInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterInventoryComponent, CurrentWeapon, COND_None, REPNOTIFY_OnChanged);
}

int32 UCharacterInventoryComponent::AddItems(const TArray<AWeapon*>& NewWeapons)
{
	const int32 NumAdded = Super::AddItems(NewWeapons);

	// If none equipped and added an item. Equip
	if(NumAdded && !GetCurrentWeapon() && Weapons.IsValidIndex(GetCurrentIndex()))
		SetCurrentWeapon(GetCurrentIndex());

	return NumAdded;
}


void UCharacterInventoryComponent::RemoveItem(const int32 Index)
{
	if(Index == GetCurrentIndex())
	{
		Super::RemoveItem(Index);
		const AWeapon* OldWeapon = CurrentWeapon;
		if(Weapons.IsValidIndex(GetCurrentIndex()))
		{
			CurrentWeapon = Weapons[GetCurrentIndex()];
			OnRep_CurrentWeapon(OldWeapon);
		}
		else if(Weapons.IsValidIndex(GetCurrentIndex() - 1))
		{
			CurrentWeapon = Weapons[GetCurrentIndex() - 1];
			CurrentIndex--;
			OnRep_CurrentWeapon(OldWeapon);
		}
		else
		{
			CurrentWeapon = nullptr;
			OnRep_CurrentWeapon(OldWeapon);
		}
	}
	else Super::RemoveItem(Index);
}


bool UCharacterInventoryComponent::SetCurrentWeapon(const int32 Index)
{
	if(!Weapons.IsValidIndex(Index)) return false;
	const AWeapon* OldWeapon = CurrentWeapon;
	CurrentWeapon = Weapons[Index];
	OnRep_CurrentWeapon(OldWeapon); // Directly calls CurrentWeaponChanged()
	return true;
}

void UCharacterInventoryComponent::CurrentWeaponChanged_Implementation(const AWeapon* OldWeapon, const int32 OldIndex)
{
	// idk why it is not being set on all instances on begin play so gonna put this here idk
	if(!OwningCharacter) OwningCharacter = CastChecked<AShooterCharacter>(GetOwner());

	// Init current index
	const int32 Index = Weapons.Find(CurrentWeapon);
	if(Index != INDEX_NONE) CurrentIndex = Index;
	
	// Remove weapon state related gameplay tags
	OwningCharacter->GetASC()->RemoveLooseGameplayTagChildren(TAG("WeaponState"));

	// Only calls weapon if valid
	CallOnUnEquipped(const_cast<AWeapon*>(OldWeapon), this);
	CallOnEquipped(CurrentWeapon, this);

	CurrentWeaponChangeDelegate.Broadcast(CurrentWeapon, OldWeapon);
}



/*
UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	SetIsReplicated(true);
}

void UCharacterInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterInventoryComponent, CurrentWeapon, COND_None, REPNOTIFY_OnChanged);
}


void UCharacterInventoryComponent::BeginPlay()
{
	OwningCharacter = CastChecked<AShooterCharacter>(GetOwner());
	
	Super::BeginPlay();
	
	// Equips DefaultItemIndex from inventory. Only functions with the equip gameplay ability
	if(OwningCharacter->HasAuthority() && Weapons.IsValidIndex(0))
		SetCurrentIndex(0);
}

int32 UCharacterInventoryComponent::AddItems(const TArray<AWeapon*>& NewWeapons)
{
	const int32 Num = Super::AddItems(NewWeapons);
	
	// If nothing equipped, equip first index of new weapons
	/*if(!OwningCharacter->GetCurrentWeapon() && !Weapons.IsValidIndex(0) && NewWeapons.IsValidIndex(0) && NewWeapons[0])
		OwningCharacter->SetCurrentWeapon(NewWeapons[0]);*/
	/*
	if(!OwningCharacter->GetCurrentWeapon() && Weapons.IsValidIndex(0))
		OwningCharacter->SetCurrentWeapon(Weapons[0]);*/
	/*if(!CurrentWeapon && Weapons.IsValidIndex(0))
		CurrentWeapon = Weapons[0];
	
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
		if(!AbilityClass) continue;
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

void UCharacterInventoryComponent::SetCurrentIndex(const int32 NewIndex)
{
	if(NewIndex == INDEX_NONE || Weapons.IsEmpty())
	{
		LastIndex = CurrentIndex;
		CurrentIndex = INDEX_NONE;
		const AWeapon* OldWeapon = CurrentWeapon;
		CurrentWeapon = nullptr;
		OnRep_CurrentWeapon(OldWeapon);
	}
	else if(Weapons.IsValidIndex(NewIndex))
	{
		LastIndex = CurrentIndex;
		CurrentIndex = NewIndex;
		const AWeapon* OldWeapon = CurrentWeapon;
		CurrentWeapon = Weapons[CurrentIndex];
		OnRep_CurrentWeapon(OldWeapon);
	}
}

void UCharacterInventoryComponent::OnRep_CurrentWeapon(const AWeapon* OldWeapon)
{
	if(OldWeapon)
	{
		OldWeapon->SetVisibility(false);
		RemoveAbilities();
	}
	if(CurrentWeapon)
	{
		CurrentWeapon->SetVisibility(true);

		
		if(OwningCharacter)
		{
			// Play third person equip montage to all instances
			if(UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance())
				if(UAnimMontage* Montage = CurrentWeapon->GetTP_EquipMontage())
					AnimInstance->Montage_Play(Montage);

			// If locally controlled, play first person equip montage
			if(OwningCharacter->IsLocallyControlled())
				if(UAnimInstance* AnimInstance = OwningCharacter->GetFP_Mesh()->GetAnimInstance())
					if(UAnimMontage* Montage = CurrentWeapon->GetFP_EquipMontage())
						AnimInstance->Montage_Play(Montage);

			if(OwningCharacter->HasAuthority())
				GiveAbilities(CurrentWeapon);
		}
		else PRINT(TEXT("OwningCharacter == NULL"));
	}
	
	CurrentWeaponChangeDelegate.Broadcast(CurrentWeapon, OldWeapon);
}

*/

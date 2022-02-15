// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "GAS/Abilities/Weapons/Rifle.h"


// friend func of AWeapon
void CallOnObtained(AWeapon* Weapon, UInventoryComponent* Inventory)
{
	if(Weapon) Weapon->OnObtained(Inventory);
}

void CallOnRemoved(AWeapon* Weapon, UInventoryComponent* Inventory)
{
	if(Weapon) Weapon->OnRemoved(Inventory);
}

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(GetOwner()->HasAuthority() && !DefaultWeapons.IsEmpty())
	{// Spawn default weapons and initialize
		//FActorSpawnParameters Params;
		//Params.Owner = GetOwner();
		TArray<AWeapon*> NewWeapons;
		for(const TSubclassOf<AWeapon>& Class : DefaultWeapons)
		{
			if(!Class) continue;
			NewWeapons.Add(GetWorld()->SpawnActor<AWeapon>(Class/*, Params*/));
		}
		
		UInventoryComponent::AddItems(NewWeapons);
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UInventoryComponent, Weapons, COND_OwnerOnly, REPNOTIFY_OnChanged);
}

int32 UInventoryComponent::AddItems(const TArray<AWeapon*>& NewWeapons)
{
	if(!GetOwner()->HasAuthority() || NewWeapons.IsEmpty()) return 0;
	
	int32 Num = 0;
	for(AWeapon* NewWeapon : NewWeapons)
	{
		if(!CanAddItem(NewWeapon)) continue;
		NewWeapon->SetOwner(GetOwner());
		//CallOnObtained(NewWeapon, this);
		Weapons.Add(NewWeapon);
		Num++;
	}
	OnRep_Weapons();

	return Num;
}

void UInventoryComponent::OnRep_Weapons()
{
	for(AWeapon* Weapon : Weapons)
	{
		if(Weapon && Weapon->GetCurrentInventory() != this)
		{
			CallOnObtained(Weapon, this);
			Weapon->SetVisibility(false);
		}
	}
}

void UInventoryComponent::RemoveItem(const int32 Index)
{
	if(Weapons.IsValidIndex(Index))
	{
		CallOnRemoved(Weapons[Index], this);
		Weapons.RemoveAt(Index);
	}

	UpdateInventoryDelegate.Broadcast();
}


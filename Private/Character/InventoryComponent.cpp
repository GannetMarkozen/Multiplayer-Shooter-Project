// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "GAS/Abilities/Weapons/Rifle.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicated(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(GetOwner()->HasAuthority())
	{
		TArray<AWeapon*> NewWeapons;
		for(const TSubclassOf<AWeapon>& Class : DefaultWeapons)
		{
			if(!Class) continue;
			FActorSpawnParameters Params;
			Params.Owner = GetOwner();
			NewWeapons.Add(GetWorld()->SpawnActor<AWeapon>(Class, Params));
		}
		AddItems(NewWeapons);
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UInventoryComponent, Weapons, COND_OwnerOnly, REPNOTIFY_OnChanged);
}

// friend func of AWeapon
void CallOnObtained(AWeapon* Weapon, UInventoryComponent* Inventory)
{
	if(Weapon) Weapon->OnObtained(Inventory);
}

void UInventoryComponent::AddItems(const TArray<AWeapon*>& NewWeapons)
{
	if(NewWeapons.Num() == 0) return;
	
	for(AWeapon* NewWeapon : NewWeapons)
	{
		NewWeapon->SetOwner(GetOwner());
		CallOnObtained(NewWeapon, this);
	}

	Weapons.Append(NewWeapons);
	UpdateInventoryDelegate.Broadcast();
}

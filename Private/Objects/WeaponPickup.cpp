// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/WeaponPickup.h"

#include "Components/SphereComponent.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Character/ShooterCharacter.h"
#include "Net/UnrealNetwork.h"


AWeaponPickup::AWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = true;
}

AWeaponPickup* AWeaponPickup::SpawnWeaponPickup(AWeapon* Weapon, const FVector& Location, const FVector& OptionalVelocity)
{
	if(!Weapon) return nullptr;
	const FTransform SpawnTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Location);
	AWeaponPickup* Pickup = Weapon->GetWorld()->SpawnActorDeferred<AWeaponPickup>(AWeaponPickup::StaticClass(), SpawnTransform, nullptr,
		Weapon->GetCurrentOwner(), ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	
	Pickup->Weapon = Weapon;
	Pickup->SetSkeletalMesh(Weapon->GetFP_Mesh()->SkeletalMesh.Get());
	
	if(OptionalVelocity != FVector())
		Pickup->OverlapSphere->AddImpulse(OptionalVelocity);
	
	UGameplayStatics::FinishSpawningActor(Pickup, Pickup->GetActorTransform());
	
	return Pickup;
}

void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	
	if(WeaponClass && HasAuthority())
	{
		Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass);
		Weapon->GetFP_Mesh()->SetVisibility(false);
		Weapon->GetTP_Mesh()->SetVisibility(false);
	}
}

void AWeaponPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponPickup, Weapon);
}


void AWeaponPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if(PropertyChangedEvent.Property == FindFProperty<FProperty>(StaticClass(), GET_MEMBER_NAME_CHECKED(AWeaponPickup, WeaponClass)))
	{
		Mesh->SetSkeletalMesh(WeaponClass ? WeaponClass.GetDefaultObject()->GetTP_Mesh()->SkeletalMesh : nullptr);
	}
}


void AWeaponPickup::Interact_Implementation(AShooterCharacter* Interactor)
{
	if(Interactor->IsLocallyControlled())
	{
		Execute_EndInspect(this, Interactor);
	}
	if(HasAuthority())
	{
		Interactor->GetCharacterInventory()->AddItem(Weapon);
		Destroy();
	}
}

void AWeaponPickup::Inspect_Implementation(AShooterCharacter* Interactor)
{
	// Applies green outline
	Mesh->SetRenderCustomDepth(true);

	// Sets HUD inspect text
	if(Interactor && Weapon)
		Interactor->HUDInspectTextDelegate.Broadcast(FText::FromString("Press E to equip " + Weapon->GetItemName().ToString()));
}

void AWeaponPickup::EndInspect_Implementation(AShooterCharacter* Interactor)
{
	// Removes green outline
	Mesh->SetRenderCustomDepth(false);

	// Clears HUD inspect text
	if(Interactor && Weapon)
		Interactor->HUDInspectTextDelegate.Broadcast(FText());
}




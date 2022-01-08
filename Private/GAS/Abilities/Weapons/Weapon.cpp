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
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	DefaultScene = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene"));
	RootComponent = DefaultScene;
	
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FP_Mesh->SetOnlyOwnerSee(true);
	FP_Mesh->SetSimulatePhysics(false);
	FP_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FP_Mesh->SetCollisionObjectType(ECC_Pawn);
	FP_Mesh->SetupAttachment(RootComponent);

	TP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	TP_Mesh->SetOwnerNoSee(true);
	TP_Mesh->SetSimulatePhysics(false);
	TP_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TP_Mesh->SetCollisionObjectType(ECC_Pawn);
	TP_Mesh->CanCharacterStepUpOn = ECB_No;
	TP_Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TP_Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	TP_Mesh->SetupAttachment(RootComponent);
	
	DamageEffect = UDamageEffect::StaticClass();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, Ammo, COND_OwnerOnly, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, ReserveAmmo, COND_OwnerOnly, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, CurrentOwner, COND_None, REPNOTIFY_OnChanged);
}

void AWeapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(AWeapon, Ammo, CurrentASC && !CurrentASC->HasMatchingGameplayTag(DelayAmmoReplicationTag));
	DOREPLIFETIME_ACTIVE_OVERRIDE(AWeapon, ReserveAmmo, CurrentASC && !CurrentASC->HasMatchingGameplayTag(DelayAmmoReplicationTag));
}

void AWeapon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
}


void AWeapon::OnObtained_Implementation(UInventoryComponent* Inventory)
{// Server only
	CurrentInventory = Inventory;
	AShooterCharacter* NewOwner = CastChecked<AShooterCharacter>(CurrentInventory ? CurrentInventory->GetOwner() : nullptr);
	if(NewOwner != CurrentOwner)
	{
		if(NewOwner)
		{// Init variables
			CurrentCharacterInventory = NewOwner->GetCharacterInventory();
			CurrentASC = NewOwner->GetASC();
		}

		// Set new owner and call onrep func
		const AShooterCharacter* OldOwner = CurrentOwner;
		CurrentOwner = NewOwner;
		OnRep_CurrentOwner(OldOwner);
	}
}

void AWeapon::OnRep_CurrentOwner_Implementation(const AShooterCharacter* OldOwner)
{
	if(CurrentOwner)
	{
		FP_Mesh->AttachToComponent(CurrentOwner->GetFP_Mesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponPoint"));
		TP_Mesh->AttachToComponent(CurrentOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponPoint"));

		FP_Mesh->SetVisibility(true);
		TP_Mesh->SetVisibility(true);

		if(FP_Mesh->SkeletalMesh && CurrentOwner->GetItemMeshDataTable())
		{
			if(const FMeshTableRow* MeshRow = CurrentOwner->GetItemMeshDataTable()->FindRow<FMeshTableRow>(FP_Mesh->SkeletalMesh->GetFName(), "MeshTableRow"))
			{
				const FTransform RelativeTransform(MeshRow->RelativeRotation, MeshRow->RelativeLocation);
				const AWeapon* DefObj = (AWeapon*)GetClass()->GetDefaultObject();
				FP_Mesh->SetRelativeTransform(DefObj->FP_Mesh->GetRelativeTransform() * RelativeTransform);
				TP_Mesh->SetRelativeTransform(DefObj->TP_Mesh->GetRelativeTransform() * RelativeTransform);
			}
		}
	}
}

void AWeapon::OnFire_Implementation()
{
	const int32 OldAmmo = Ammo--;
	OnRep_Ammo(OldAmmo);/*
	if(CurrentASC && HasAuthority() && !CurrentOwner->IsLocallyControlled())
		CurrentASC->AddLooseGameplayTagForDuration(DelayAmmoReplicationTag, RateOfFire + 0.1f);*/
}



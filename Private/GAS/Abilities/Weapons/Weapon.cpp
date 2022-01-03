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
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FP_Mesh->SetOnlyOwnerSee(true);
	FP_Mesh->SetSimulatePhysics(false);
	FP_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FP_Mesh->SetCollisionObjectType(ECC_Pawn);

	TP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	TP_Mesh->SetOwnerNoSee(true);
	TP_Mesh->SetSimulatePhysics(false);
	TP_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TP_Mesh->SetCollisionObjectType(ECC_Pawn);
	TP_Mesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	TP_Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	DamageEffect = UDamageEffect::StaticClass();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, Ammo, COND_OwnerOnly, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, CurrentOwner, COND_None, REPNOTIFY_OnChanged);
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
		for(USkeletalMeshComponent* Mesh : {FP_Mesh, TP_Mesh})
		{
			Mesh->SetVisibility(true);
			if(Mesh->SkeletalMesh)
				if(const FMeshTableRow* MeshRow = CurrentOwner->GetItemMeshDataTable()->FindRow<FMeshTableRow>(Mesh->SkeletalMesh->GetFName(), "MeshTableRow"))
					Mesh->SetRelativeTransform(FTransform(MeshRow->RelativeRotation, MeshRow->RelativeLocation));
		}
	}
}


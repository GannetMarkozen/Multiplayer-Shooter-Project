// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Weapons/Weapon.h"

#include "Character/ShooterCharacter.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/GASGameplayAbility.h"
#include "GAS/AttributeSets/AmmoAttributeSet.h"
#include "GAS/Effects/DamageEffect.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AmmoAttribute = UAmmoAttributeSet::GetRifleAmmoAttribute();

	DefaultScene = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene"));
	RootComponent = DefaultScene;
	
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FP_Mesh->SetOnlyOwnerSee(true);
	FP_Mesh->SetCastShadow(false);
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
	TP_Mesh->SetCollisionResponseToChannel(ECC_ItemDrop, ECR_Ignore);
	TP_Mesh->SetCollisionResponseToChannel(ECC_Projectile, ECR_Ignore);
	TP_Mesh->SetupAttachment(RootComponent);
	
	DamageEffect = UDamageEffect::StaticClass();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, Ammo, COND_OwnerOnly, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, CurrentInventory, COND_None, REPNOTIFY_OnChanged);
}

void AWeapon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
}

void AWeapon::OnObtained_Implementation(UInventoryComponent* Inventory)
{
	if(Inventory != CurrentInventory)
	{
		const UInventoryComponent* OldInventory = CurrentInventory;
		CurrentInventory = Inventory;
		OnRep_CurrentInventory(OldInventory);
	}
}

void AWeapon::OnRemoved_Implementation(UInventoryComponent* Inventory)
{
	CurrentInventory = nullptr;
	OnRep_CurrentInventory(Inventory);
}


void AWeapon::OnRep_CurrentInventory_Implementation(const UInventoryComponent* OldInventory)
{
	if(CurrentInventory)
	{
		// If owner is AShooterCharacter, attach and init vars
		if((CurrentOwner = Cast<AShooterCharacter>(CurrentInventory->GetOwner())) != nullptr)
		{
			CurrentCharacterInventory = CurrentOwner->GetCharacterInventory();
			CurrentASC = CurrentOwner->GetASC();
			
			// Attach weapon mesh to character
			FP_Mesh->AttachToComponent(CurrentOwner->GetFP_Mesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponPoint"));
			TP_Mesh->AttachToComponent(CurrentOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponPoint"));

			// Invisible if not currently equipped
			if(CurrentOwner->GetCurrentWeapon() != this)
				SetVisibility(false);

			// Set weapon mesh orientation
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
			
			// Bind reserve ammo delegate on changed
			if(AmmoAttribute.IsValid())
				CurrentASC->GetGameplayAttributeValueChangeDelegate(AmmoAttribute).AddUObject(this, &AWeapon::ReserveAmmoUpdated);
		}
	}
	else
	{
		// Unbind reserve ammo delegate
		if(AmmoAttribute.IsValid() && CurrentASC)
			CurrentASC->GetGameplayAttributeValueChangeDelegate(AmmoAttribute).RemoveAll(this);
		
		CurrentOwner = nullptr;
		CurrentCharacterInventory = nullptr;
		CurrentASC = nullptr;
	}
}


/*
void AWeapon::OnObtained_Implementation(UInventoryComponent* Inventory)
{// Server only
	CurrentInventory = Inventory;
	AShooterCharacter* NewOwner = CastChecked<AShooterCharacter>(CurrentInventory ? CurrentInventory->GetOwner() : nullptr);
	if(NewOwner != CurrentOwner)
	{
		// Set new owner and call onrep func
		const AShooterCharacter* OldOwner = CurrentOwner;
		CurrentOwner = NewOwner;
		OnRep_CurrentOwner(OldOwner);
	}
}

void AWeapon::OnRemoved_Implementation(UInventoryComponent* Inventory)
{// Server only
	const AShooterCharacter* OldOwner = CurrentOwner;
	CurrentInventory = nullptr;
	CurrentOwner = nullptr;
	OnRep_CurrentOwner(OldOwner);
}

void AWeapon::OnRep_CurrentOwner_Implementation(const AShooterCharacter* OldOwner)
{// All instances
	if(CurrentOwner)
	{
		// Init variables
		CurrentCharacterInventory = CurrentOwner->GetCharacterInventory();
		CurrentASC = CurrentOwner->GetASC();
		
		// Attach weapon mesh to character
		FP_Mesh->AttachToComponent(CurrentOwner->GetFP_Mesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponPoint"));
		TP_Mesh->AttachToComponent(CurrentOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponPoint"));

		// Set weapon mesh orientation
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

		if(AmmoAttribute.IsValid())
			CurrentASC->GetGameplayAttributeValueChangeDelegate(AmmoAttribute).AddUObject(this, &AWeapon::ReserveAmmoUpdated);
	}
	else
	{
		if(AmmoAttribute.IsValid() && CurrentASC)
			CurrentASC->GetGameplayAttributeValueChangeDelegate(AmmoAttribute).RemoveAll(this);
		
		CurrentCharacterInventory = nullptr;
		CurrentASC = nullptr;
	}
}*/


void AWeapon::OnFire_Implementation()
{
	SetAmmo(Ammo - 1);
	//if(CurrentOwner->IsLocallyControlled())
	//CurrentASC->AddLooseGameplayTagForDurationSingle(TAG("WeaponState.Firing"), RateOfFire);
}

FGameplayAttributeData* AWeapon::GetAmmoAttributeData() const
{
	if(AmmoAttribute.IsValid() && CurrentASC)
		return AmmoAttribute.GetUProperty()->ContainerPtrToValuePtr<FGameplayAttributeData>((UAmmoAttributeSet*)CurrentASC->GetSet<UAmmoAttributeSet>());
	return nullptr;
}

void AWeapon::SetReserveAmmo(const int32 NewReserveAmmo)
{
	if(AmmoAttribute.IsValid() && CurrentASC)
		GetAmmoAttributeData()->SetCurrentValue(NewReserveAmmo);
}

int32 AWeapon::GetReserveAmmo() const
{
	return AmmoAttribute.IsValid() && CurrentASC ? GetAmmoAttributeData()->GetCurrentValue() : 0;
}




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

	WeaponAttachmentSocketName = FName("weaponsocket_r");
	AmmoAttribute = UAmmoAttributeSet::GetRifleAmmoAttribute();

	DefaultScene = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene"));
	RootComponent = DefaultScene;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionObjectType(ECC_Pawn);
	Mesh->CanCharacterStepUpOn = ECB_No;
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	Mesh->SetCollisionResponseToChannel(ECC_ItemDrop, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Projectile, ECR_Ignore);
	Mesh->SetupAttachment(DefaultScene);
	
	DamageEffect = UDamageEffect::StaticClass();
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}


void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, Ammo, COND_OwnerOnly, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, CurrentInventory, COND_None, REPNOTIFY_OnChanged);
}

void AWeapon::OnObtained(UInventoryComponent* Inventory)
{
	if(Inventory != CurrentInventory)
	{
		const UInventoryComponent* OldInventory = CurrentInventory;
		CurrentInventory = Inventory;
		OnRep_CurrentInventory(OldInventory);
		
		BP_OnObtained(Inventory);
	}
}

void AWeapon::OnRemoved(UInventoryComponent* Inventory)
{
	RemoveAbilities();
	
	CurrentInventory = nullptr;
	OnRep_CurrentInventory(Inventory);

	BP_OnRemoved(Inventory);
}

void AWeapon::OnEquipped(UCharacterInventoryComponent* Inventory)
{
	SetVisibility(true);

	// Only runs on server
	GiveAbilities();

	// Sometimes current owner isn't initialized yet at this point for some reason
	if(!CurrentOwner)
		CurrentOwner = CastChecked<AShooterCharacter>(Inventory->GetOwner());
	
	if(EquipMontage)
		if(UAnimInstance* AnimInstance = CurrentOwner->GetMesh()->GetAnimInstance())
			AnimInstance->Montage_Play(EquipMontage);

	BP_OnEquipped(Inventory);
}

void AWeapon::OnUnEquipped(UCharacterInventoryComponent* Inventory)
{
	SetVisibility(false);

	// Only runs on server
	RemoveAbilities();

	BP_OnUnEquipped(Inventory);
}

void AWeapon::GiveAbilities()
{
	if(HasAuthority() && CurrentOwner)
		for(const TSubclassOf<UGASGameplayAbility>& Ability : WeaponAbilities)
			if(Ability)
				ActiveAbilities.Add(CurrentASC->GiveAbility(FGameplayAbilitySpec(Ability, 1, (int32)Ability.GetDefaultObject()->Input)));
}

void AWeapon::RemoveAbilities()
{
	if(HasAuthority() && CurrentOwner)
	{
		for(const FGameplayAbilitySpecHandle& Handle : ActiveAbilities)
		{
			//CurrentASC->CancelAbilityHandle(Handle);
			if(UGASGameplayAbility* Ability = CurrentASC->GetAbilityFromHandle(Handle))
				Ability->ExternalEndAbility();
			CurrentASC->SetRemoveAbilityOnEnd(Handle);
		}
	}
	ActiveAbilities.Empty();
}

void AWeapon::OnRep_CurrentInventory_Implementation(const UInventoryComponent* OldInventory)
{
	if(CurrentInventory)
	{
		// If owner is AShooterCharacter, attach and init vars
		CurrentOwner = Cast<AShooterCharacter>(CurrentInventory->GetOwner());
		if(CurrentOwner)
		{
			// Init vars
			CurrentCharacterInventory = CurrentOwner->GetCharacterInventory();
			CurrentASC = CurrentOwner->GetASC();

			// Invisible if not currently equipped
			if(CurrentCharacterInventory->GetCurrentWeapon() != this)
				SetVisibility(false);

			// Attach to weapon socket point
			AttachToWeaponSocket();
			
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
		SetOwner(nullptr);
	}
}

void AWeapon::AttachToWeaponSocket()
{
	if(!CurrentOwner) return;
	
	// Attach weapon mesh to character
	Mesh->AttachToComponent(CurrentOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachmentSocketName);
	
	if(Mesh->SkeletalMesh && CurrentOwner->ItemMeshDataTable)
	{
		if(const FMeshTableRow* MeshRow = CurrentOwner->ItemMeshDataTable->FindRow<FMeshTableRow>(Mesh->SkeletalMesh->GetFName(), "MeshTableRow on AWeapon"))
		{
			const AWeapon* DefObj = (AWeapon*)GetClass()->GetDefaultObject();
			const FTransform RelativeTransform(MeshRow->RelativeRotation, MeshRow->RelativeLocation);
			Mesh->SetRelativeTransform(DefObj->Mesh->GetRelativeTransform() * RelativeTransform);
		}
	}
}

FGameplayAttributeData* AWeapon::GetAmmoAttributeData() const
{
	if(AmmoAttribute.IsValid() && CurrentASC)
		return AmmoAttribute.GetUProperty()->ContainerPtrToValuePtr<FGameplayAttributeData>((UAmmoAttributeSet*)CurrentASC->GetSet<UAmmoAttributeSet>());
	return nullptr;
}

void AWeapon::SetReserveAmmo(const int32 NewReserveAmmo)
{
	if(!AmmoAttribute.IsValid() || !CurrentASC) return;
	
	// Create runtime GE to override reserve ammo
	UGameplayEffect* GameplayEffect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("RuntimeInstantGE"));
	GameplayEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

	const int32 Idx = GameplayEffect->Modifiers.Num();
	GameplayEffect->Modifiers.SetNum(Idx + 1);
	FGameplayModifierInfo& ModifierInfo = GameplayEffect->Modifiers[Idx];
	ModifierInfo.Attribute = AmmoAttribute;
	ModifierInfo.ModifierMagnitude = FScalableFloat(NewReserveAmmo);
	ModifierInfo.ModifierOp = EGameplayModOp::Override;

	const FGameplayEffectSpec* Spec = new FGameplayEffectSpec(GameplayEffect, {}, 1.f);
	CurrentASC->ApplyGameplayEffectSpecToSelf(*Spec);
}

int32 AWeapon::GetReserveAmmo() const
{
	return AmmoAttribute.IsValid() && CurrentASC ? GetAmmoAttributeData()->GetCurrentValue() : 0;
}




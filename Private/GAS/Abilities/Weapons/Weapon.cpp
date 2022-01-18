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
	RemoveAbilities();
	
	CurrentInventory = nullptr;
	OnRep_CurrentInventory(Inventory);
}

void AWeapon::OnEquipped_Implementation(UCharacterInventoryComponent* Inventory)
{
	SetVisibility(true);

	// Only runs on server
	GiveAbilities();

	// Sometimes current owner isn't initialized yet at this point for some reason
	if(!CurrentOwner)
		CurrentOwner = CastChecked<AShooterCharacter>(Inventory->GetOwner());
	
	if(TP_EquipMontage)
		if(UAnimInstance* AnimInstance = CurrentOwner->GetMesh()->GetAnimInstance())
			AnimInstance->Montage_Play(TP_EquipMontage);

	if(CurrentOwner->IsLocallyControlled() && FP_EquipMontage)
		if(UAnimInstance* AnimInstance = CurrentOwner->GetFP_Mesh()->GetAnimInstance())
			AnimInstance->Montage_Play(FP_EquipMontage);
}

void AWeapon::OnUnEquipped_Implementation(UCharacterInventoryComponent* Inventory)
{
	SetVisibility(false);

	// Only runs on server
	RemoveAbilities();
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
	FP_Mesh->AttachToComponent(CurrentOwner->GetFP_Mesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachmentSocketName);
	TP_Mesh->AttachToComponent(CurrentOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachmentSocketName);

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




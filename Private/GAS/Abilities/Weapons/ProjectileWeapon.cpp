// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Weapons/ProjectileWeapon.h"

#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "Components/AudioComponent.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/Abilities/Weapons/RecoilInstance.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"


AProjectileWeapon::AProjectileWeapon()
{
	AmmoAttribute = UAmmoAttributeSet::GetRifleAmmoAttribute();

	
}

void AProjectileWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AProjectileWeapon, Ammo, COND_OwnerOnly, REPNOTIFY_OnChanged);
}


void AProjectileWeapon::OnRep_CurrentInventory_Implementation(const UInventoryComponent* OldInventory)
{
	Super::OnRep_CurrentInventory_Implementation(OldInventory);
	
	if(CurrentInventory)
	{// Bind reserve ammo delegate on changed
		if(AmmoAttribute.IsValid())
			CurrentASC->GetGameplayAttributeValueChangeDelegate(AmmoAttribute).AddUObject(this, &AProjectileWeapon::ReserveAmmoUpdated);
	}
	else
	{// Unbind reserve ammo delegate
		if(AmmoAttribute.IsValid() && CurrentASC)
			CurrentASC->GetGameplayAttributeValueChangeDelegate(AmmoAttribute).RemoveAll(this);
	}
}

FGameplayAttributeData* AProjectileWeapon::GetAmmoAttributeData() const
{
	return CurrentOwner && AmmoAttribute.IsValid() ? AmmoAttribute.GetUProperty()->ContainerPtrToValuePtr<FGameplayAttributeData>(CurrentOwner->AmmoSet) : nullptr;
}


void AProjectileWeapon::SetReserveAmmo(const int32 NewReserveAmmo)
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

void AProjectileWeapon::OnFireWeapon_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
{
	const FTransform& MuzzleTransform = GetMuzzleWorldTransform();
	
	if(FiringSound)
		UGameplayStatics::SpawnSoundAttached(FiringSound, Mesh, FName("Muzzle"), MuzzleTransform.GetLocation(), FRotator::ZeroRotator, EAttachLocation::KeepWorldPosition);

	if(MuzzleFlash)
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Mesh, FName("Muzzle"), MuzzleTransform.GetLocation(), MuzzleTransform.Rotator(), FVector(1.f), EAttachLocation::KeepWorldPosition);

	if(RecoilClass)
		URecoilInstance::AddRecoilInstance(CurrentOwner, RecoilClass, 1.f);
}

void AProjectileWeapon::OnFireWeaponEnd_Implementation()
{
	
}

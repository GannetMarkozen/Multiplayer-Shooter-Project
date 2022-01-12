// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/AmmoAttributeSet.h"
#include "Net/UnrealNetwork.h"


UAmmoAttributeSet::UAmmoAttributeSet()
{
	
}

void UAmmoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, RifleAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, ShotgunAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, PistolAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, ExplosiveAmmo, COND_None, REPNOTIFY_Always);
}

void UAmmoAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	NewValue = FMath::Clamp<float>(NewValue, 0.f, 999.f);
}


void UAmmoAttributeSet::OnRep_RifleAmmo(const FGameplayAttributeData& OldRifleAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, RifleAmmo, OldRifleAmmo);
}

void UAmmoAttributeSet::OnRep_ShotgunAmmo(const FGameplayAttributeData& OldShotgunAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, ShotgunAmmo, OldShotgunAmmo);
}

void UAmmoAttributeSet::OnRep_PistolAmmo(const FGameplayAttributeData& OldPistolAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, PistolAmmo, OldPistolAmmo);
}

void UAmmoAttributeSet::OnRep_ExplosiveAmmo(const FGameplayAttributeData& OldExplosiveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, ExplosiveAmmo, OldExplosiveAmmo);
}


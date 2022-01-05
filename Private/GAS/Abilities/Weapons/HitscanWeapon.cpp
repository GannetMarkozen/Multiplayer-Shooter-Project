// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Weapons/HitscanWeapon.h"

#include "GAS/Abilities/HitscanAbility.h"
#include "GAS/GASBlueprintFunctionLibrary.h"

AHitscanWeapon::AHitscanWeapon()
{
	WeaponAbilities.Add(UHitscanAbility::StaticClass());
	bUseAmmo = true;
}

int32 AHitscanWeapon::CalculateDamage_Implementation(const AActor* Target, const FGameplayEffectSpecHandle& Spec) const
{
	if(DamageFalloffCurve && Target && Spec.Data.IsValid() && Spec.Data.Get()->GetEffectContext().IsValid())
	{
		const int32 NumHits = GAS::GetNumCompoundingHits(Target, ((FGameplayEffectContextExtended*)Spec.Data.Get()->GetEffectContext().Get())->GetTargetData());
		if(NumHits > 0)
		{
			const FVector& From = Spec.Data.Get()->GetEffectContext().Get()->GetInstigator()->GetActorLocation();
			const FVector& To = Target->GetActorLocation();
			const float Distance = FVector::Distance(From, To);
			return BaseDamage * DamageFalloffCurve.Get()->GetFloatValue(Distance / EffectiveRange) * NumHits;
		}
	}
	return 0;
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Weapons/HitscanWeapon.h"

#include "Character/ShooterCharacter.h"
#include "GAS/Abilities/Firing/HitscanAbility.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/Abilities/ReloadWeaponAbility.h"

AHitscanWeapon::AHitscanWeapon()
{
	WeaponAbilities.Add(UHitscanAbility::StaticClass());
	WeaponAbilities.Add(UReloadWeaponAbility::StaticClass());
}

void AHitscanWeapon::SetupInputBindings()
{
	Super::SetupInputBindings();

	SetupBind(this, &AHitscanWeapon::FireWeapon, EInputBinding::PrimaryFire, IE_Pressed);
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
			return BaseDamage * DamageFalloffCurve->GetFloatValue(Distance / EffectiveRange) * NumHits;
		}
	}
	return 0;
}

void AHitscanWeapon::OnFireWeapon_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
{
	Super::OnFireWeapon_Implementation(TargetData);

	if(BulletTracer)
	{
		TArray<FHitResult> Hits;
		GAS::GetHitArray(TargetData, Hits);
		
		for(const FHitResult& Hit : Hits)
		{
			constexpr float MinDistance = 60.f;
			if(Hit.Distance < MinDistance) continue;

			const FVector& MuzzleLocation = GetMuzzleWorldTransform().GetLocation();
			const float Distance = FMath::FRandRange(MinDistance, FMath::Min<float>(Hit.Distance - MinDistance, 1000.f));
			const FQuat& Rotation = (Hit.Location - MuzzleLocation).ToOrientationQuat();
			const FTransform TracerTransform(Rotation, MuzzleLocation + Rotation.Vector() * Distance);
			
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletTracer, TracerTransform);
		}
	}
}

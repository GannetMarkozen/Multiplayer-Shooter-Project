
#include "GAS/Abilities/Weapons/FireModes/HitscanFireModes/HitscanFireModes.h"

#include "Camera/CameraComponent.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/Abilities/Weapons/RecoilInstance.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

void UHitscanFiringObject::Hitscan()
{
	if(!CanHitscan()) return;

	const FVector& Start = GetCharacter()->GetCamera()->GetComponentLocation();
	const FRotator& BaseAimRotation = GetCharacter()->GetBaseAimRotation();
	const FVector2D& ScaledSpread = Weapon->GetFiringSpread() * Weapon->CalculateFiringSpreadMagnitude();

	FGameplayAbilityTargetDataHandle TargetData;
	// Fire for the amount of shots per round
	for(int32 i = 0; i < Weapon->GetNumShots(); i++)
	{
		// Calc random spread each iteration
		const FVector2D& RandomSpread = FVector2D(FMath::FRandRange(-ScaledSpread.X, ScaledSpread.X), FMath::FRandRange(-ScaledSpread.Y, ScaledSpread.Y));
		const FRotator& Rotation = FRotator(BaseAimRotation.Pitch + RandomSpread.Y, BaseAimRotation.Yaw + RandomSpread.X, 0.f);
		const FVector& End = Start + Rotation.Vector() * Weapon->GetRange();
        	
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActors(TArray<AActor*>({Weapon, GetCharacter()}));
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Projectile, Params);
		if(Hit.IsValidBlockingHit())
		{
			TargetData.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
		}
		else
		{
			Hit.Distance = Weapon->GetRange();
			Hit.Location = End;
			TargetData.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
		}
	}

	FGameplayEffectContextHandle Context = GAS::MakeEffectContextHandle(GetASC(), nullptr, TargetData, FGameplayTagContainer::EmptyContainer);

	Weapon->DecrementAmmo();
	LocalModifyContext(Context);
	
	OnFireWeapon(Context);
	Server_Hitscan(Context);
}

void UHitscanFiringObject::Server_Hitscan_Implementation(const FGameplayEffectContextHandle& Context)
{
	if(!Weapon)
	{
		Weapon = GetTypedOuter<AHitscanWeapon>();
		check(Weapon);
	}

	if(!GetCharacter()->IsLocallyControlled() && !CanHitscan())
	{// Check if server mis-predicted
		Weapon->UpdateClientAmmo();
		return;
	}

	// Make effect spec. This will be used in the damage logic
	const FGameplayEffectSpecHandle& Spec = GetASC()->MakeOutgoingSpec(Weapon->GetDamageEffect(), 1.f, Context);
	Spec.Data.Get()->DynamicAssetTags.AppendTags(Weapon->GetDamageCalculationTags());

	// Get target data. This should hold our hit results
	const FGameplayAbilityTargetDataHandle& TargetData = GAS::GetTargetDataHandle(Context);
	
	TArray<FHitResult> Hits;
	GAS::GetHitArray(TargetData, Hits);

	// Apply knockback to knockbackable to targets
	TArray<AActor*> KnockbackTargets;
	for(const FHitResult& Hit : Hits)
	{
		if(Hit.GetActor() && GAS::ShouldProjectileApplyForce(Hit.GetComponent()) && KnockbackTargets.Find(Hit.GetActor()) == INDEX_NONE)
		{
			FGameplayCueParameters Params(FGameplayEffectContextHandle(new FGameplayEffectContextExtended(GetCharacter(), GAS::FilterTargetDataByActor(Hit.GetActor(), TargetData))));
			Params.RawMagnitude = IDamageCalculationInterface::Execute_CalculateDamage(Weapon, Hit.GetActor(), Spec);
			GetASC()->NetMulticast_InvokeGameplayCueExecuted_WithParams(TAG("GameplayCue.Knockback"), FPredictionKey(), Params);
			KnockbackTargets.Add(Hit.GetActor());
		}
	}

	// Apply damage to damageable targets
	TArray<AActor*> DamageableTargets;
	for(const FHitResult& Hit : Hits)
	{
		if(Hit.GetActor() && Hit.GetActor()->Implements<UDamageInterface>() && DamageableTargets.Find(Hit.GetActor()) == INDEX_NONE)
		{
			IDamageInterface::ApplyDamage(GetASC(), Hit.GetActor(), Spec);
		}
	}
	
	if(!GetCharacter()->IsLocallyControlled())
		OnFireWeapon(Context);
	Multi_CallOnFireWeapon(Context);
}

void UHitscanFiringObject::OnFireWeapon_Implementation(const FGameplayEffectContextHandle& Context)
{
	const FTransform& MuzzleWorldTransform = Weapon->GetMuzzleWorldTransform();
	
	SpawnFiringSound(Context, MuzzleWorldTransform);
	SpawnMuzzleFlash(Context, MuzzleWorldTransform);	
	SpawnRecoilInstance(Context, MuzzleWorldTransform);
	SpawnBulletTracers(Context, MuzzleWorldTransform);
	
	/*if(BulletTracer)
	{
		TArray<FHitResult> Hits;
		GAS::GetHitArray(TargetData, Hits);
		
		for(int32 i = 0; i < Hits.Num(); i++)
		{
			constexpr float MinDistance = 0.f;
			if(Hits[i].Distance < MinDistance) continue;

			const FVector& MuzzleLocation = MuzzleTransform.GetLocation();
			const float Distance = FMath::FRandRange(MinDistance, FMath::Min<float>(Hits[i].Distance - MinDistance, 1000.f));
			const FQuat& Rotation = (Hits[i].Location - MuzzleLocation).ToOrientationQuat();
			const FTransform TracerTransform(Rotation, MuzzleLocation + Rotation.Vector() * Distance);

			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletTracer, TracerTransform);
		}
	}*/
}

void UHitscanFiringObject::SpawnBulletTracers_Implementation(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform)
{
	if(BulletTracer)
	{
		TArray<FHitResult> Hits;
		GAS::GetHitArray(GAS::GetTargetDataHandle(Context), Hits);
		
		for(int32 i = 0; i < Hits.Num(); i++)
		{
			constexpr float MinDistance = 0.f;
			if(Hits[i].Distance < MinDistance) continue;

			const FVector& MuzzleLocation = MuzzleWorldTransform.GetLocation();
			const float Distance = FMath::FRandRange(MinDistance, FMath::Min<float>(Hits[i].Distance - MinDistance, 1000.f));
			const FQuat& Rotation = (Hits[i].Location - MuzzleLocation).ToOrientationQuat();
			const FTransform TracerTransform(Rotation, MuzzleLocation + Rotation.Vector() * Distance);

			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletTracer, TracerTransform);
		}
	}
}



/*
 *	SEMI-AUTO
 */

void USemiAutoFiringObject::StartFiring_Implementation()
{
	Super::StartFiring_Implementation();
	if(!CanFire()) return;
	Hitscan();
	GetASC()->AddLooseGameplayTagForDurationSingle(FiringStateTag, RateOfFire);
}

/*
 *	BURST-FIRE
 */

const FGameplayTag UBurstFiringObject::LastShotTag = TAG("WeaponState.Firing");

void UBurstFiringObject::StartFiring_Implementation()
{
	Super::StartFiring_Implementation();
	if(!CanFire()) return;
	GetWorld()->GetTimerManager().SetTimer(FiringTimerHandle, this, &UBurstFiringObject::Burst_Hitscan, BurstRateOfFire, true, 0.f);
	GetASC()->AddLooseGameplayTagForDurationSingle(FiringStateTag, (NumShotsPerBurst - 1) * BurstRateOfFire + RateOfFire);
}


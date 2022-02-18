// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Weapons/HitscanWeapon.h"

#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "Engine/ActorChannel.h"
#include "GAS/Abilities/Firing/HitscanAbility.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/Abilities/ReloadWeaponAbility.h"
#include "GAS/Abilities/Weapons/FireModes/HitscanFireModes/HitscanFireModes.h"

const FGameplayTag AHitscanWeapon::FiringStateTag = TAG("WeaponState.Firing");

AHitscanWeapon::AHitscanWeapon()
{
	//WeaponAbilities.Add(UHitscanAbility::StaticClass());
	WeaponAbilities.Add(UReloadWeaponAbility::StaticClass());
}

bool AHitscanWeapon::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{// Replicate the HitscanObject for net multicast to be called
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	bWroteSomething |= Channel->ReplicateSubobject(HitscanObject, *Bunch, *RepFlags);
	return bWroteSomething;
}


void AHitscanWeapon::SetupInputBindings()
{
	Super::SetupInputBindings();

	if(HitscanObject)
	{
		SetupBind<UFiringObject>(HitscanObject, &UHitscanFiringObject::StartFiring, EInputBinding::PrimaryFire, IE_Pressed);
		SetupBind<UFiringObject>(HitscanObject, &UHitscanFiringObject::StopFiring, EInputBinding::PrimaryFire, IE_Released);
	}
}

void AHitscanWeapon::RemoveInputBindings()
{
	Super::RemoveInputBindings();
/*
	if(HitscanObject)
		UInputBinding::RemoveAllInputUObject(CurrentOwner, HitscanObject);*/
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

bool AHitscanWeapon::CanFireWeapon_Implementation() const
{
	return Ammo > 0 && CurrentOwner && !CurrentASC->HasAnyMatchingGameplayTags(FiringBlockedTags);
}


void AHitscanWeapon::StartFiring()
{
	if(FireMode == EFireMode::SemiAuto)
	{
		if(CurrentASC->HasMatchingGameplayTag(FiringStateTag)) return;
		Hitscan();
		CurrentASC->AddLooseGameplayTagForDurationSingle(FiringStateTag, RateOfFire);
	}
	else if(FireMode == EFireMode::FullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(FiringTimerHandle, this, &AHitscanWeapon::FullAuto_Hitscan, RateOfFire, true, 0.f);
	}
	else if(FireMode == EFireMode::Burst)
	{
		if(CurrentASC->HasMatchingGameplayTag(FiringStateTag)) return;
		GetWorld()->GetTimerManager().SetTimer(FiringTimerHandle, this, &AHitscanWeapon::Burst_Hitscan, BurstRateOfFire, true, 0.f);
		CurrentASC->AddLooseGameplayTagForDuration(FiringStateTag, (NumShotsPerBurst - 1) * BurstRateOfFire + RateOfFire);
	}
}

void AHitscanWeapon::StopFiring()
{
	if(FireMode == EFireMode::FullAuto)
	{
		GetWorld()->GetTimerManager().ClearTimer(FiringTimerHandle);
	}
}



void AHitscanWeapon::Hitscan()
{
	if(!CanFireWeapon()) return;
	
	const FVector& Start = CurrentOwner->GetCamera()->GetComponentLocation();
	const FRotator& BaseAimRotation = CurrentOwner->GetBaseAimRotation();
	const FVector2D& ScaledSpread = FiringSpread * CalculateFiringSpreadMagnitude();

	FGameplayAbilityTargetDataHandle TargetData;
	// Fire for the amount of shots per round
	for(int32 i = 0; i < NumShots; i++)
	{
		// Calc random spread each iteration
		const FVector2D& RandomSpread = FVector2D(FMath::FRandRange(-ScaledSpread.X, ScaledSpread.X), FMath::FRandRange(-ScaledSpread.Y, ScaledSpread.Y));
		const FRotator& Rotation = FRotator(BaseAimRotation.Pitch + RandomSpread.Y, BaseAimRotation.Yaw + RandomSpread.X, 0.f);
		const FVector& End = Start + Rotation.Vector() * Range;
        	
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActors(TArray<AActor*>({this, CurrentOwner}));
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Projectile, Params);
		if(Hit.IsValidBlockingHit())
		{
			TargetData.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
		}
		else
		{
			Hit.Distance = Range;
			Hit.Location = End;
			TargetData.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
		}
	}
	
	DecrementAmmo();
	OnFireWeapon(TargetData);
	Server_Hitscan(TargetData);
}

void AHitscanWeapon::Server_Hitscan_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if(!CurrentOwner->IsLocallyControlled() && !CanFireWeapon())
	{// Update the client's predicted ammo usage
		UpdateClientAmmo();
		return;
	}

	TArray<FHitResult> Hits;
	GAS::GetHitArray(TargetData, Hits);

	const FGameplayEffectContextHandle& Context = CurrentASC->MakeEffectContextExtended(nullptr, TargetData);
	const FGameplayEffectSpecHandle& Spec = CurrentASC->MakeOutgoingSpec(DamageEffect, 1.f, Context);
	Spec.Data.Get()->DynamicAssetTags.AppendTags(GetDamageCalculationTags());

	// Apply knockback to knockbackable to targets
	TArray<AActor*> KnockbackTargets;
	for(const FHitResult& Hit : Hits)
	{
		if(Hit.GetActor() && GAS::ShouldProjectileApplyForce(Hit.GetComponent()) && KnockbackTargets.Find(Hit.GetActor()) == INDEX_NONE)
		{
			FGameplayCueParameters Params(FGameplayEffectContextHandle(new FGameplayEffectContextExtended(CurrentOwner, GAS::FilterTargetDataByActor(Hit.GetActor(), TargetData))));
			Params.RawMagnitude = IDamageCalculationInterface::Execute_CalculateDamage(this, Hit.GetActor(), Spec);
			CurrentASC->NetMulticast_InvokeGameplayCueExecuted_WithParams(TAG("GameplayCue.Knockback"), FPredictionKey(), Params);
			KnockbackTargets.Add(Hit.GetActor());
		}
	}

	// Apply damage to damageable targets
	TArray<AActor*> DamageableTargets;
	for(const FHitResult& Hit : Hits)
	{
		if(Hit.GetActor() && Hit.GetActor()->Implements<UDamageInterface>() && DamageableTargets.Find(Hit.GetActor()) == INDEX_NONE)
		{
			IDamageInterface::ApplyDamage(CurrentASC, Hit.GetActor(), Spec);
		}
	}

	OnFireWeapon(TargetData);
	Multi_CallOnFireWeapon_SkipLocal(TargetData);
}



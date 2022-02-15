// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Firing/HitscanAbility.h"

#include "DrawDebugHelpers.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/Abilities/Weapons/HitscanWeapon.h"
#include "GAS/Effects/DamageEffect.h"

UHitscanAbility::UHitscanAbility()
{
	DamageEffectClass = UDamageEffect::StaticClass();
	
	FiringStateTag = TAG("WeaponState.Firing");
	LocalFiringCue = TAG("GameplayCue.FireWeapon.Local");
	NetMulticastFiringCue = TAG("GameplayCue.FireWeapon.NetMulticast");
	NetMulticastKnockbackCue = TAG("GameplayCue.Knockback");
	NetMulticastImpactCue = TAG("GameplayCue.Impact.Bullet");
	
	Input = EAbilityInput::PrimaryFire;
	bReplicateInputDirectly = false;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Equipping"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Reloading"));
}

void UHitscanAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// Current weapon should be valid on server by this point so we can set it here
	if(ActorInfo->IsNetAuthority())
		CurrentWeapon = CURRENTWEAPONTYPE(AHitscanWeapon);
	
	if(ActorInfo->IsNetAuthority() && !ActorInfo->IsLocallyControlled())
	{// If server and not locally controlled, setup RPC delegates
		GET_ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericSignalFromClient, Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UHitscanAbility::Server_ReceivedEvent);
		GET_ASC->AbilityTargetDataSetDelegate(Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UHitscanAbility::Server_ReceivedTargetData);
	}
}

void UHitscanAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if(ActorInfo->IsNetAuthority() && !ActorInfo->IsLocallyControlled())
	{// If server and not locally controlled, setup RPC delegates
		GET_ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericSignalFromClient, Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).RemoveAll(this);
		GET_ASC->AbilityTargetDataSetDelegate(Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).RemoveAll(this);
	}
}


bool UHitscanAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{// Must be able to fire to activate ability
	const AHitscanWeapon* TempCurrentWeapon = CurrentWeapon ? CurrentWeapon : CURRENTWEAPONTYPE(AHitscanWeapon);
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && TempCurrentWeapon && TempCurrentWeapon->CanFire();
}

void UHitscanAbility::DoLineTrace_Implementation(FHitResult& Hit, const FVector& Location, const FRotator& Rotation, const TArray<AActor*>& IgnoreActors)
{
	const FVector2D& Spread = CurrentWeapon->CalculateSpread();
	const FRotator AimRotation(Rotation.Pitch + Spread.Y, Rotation.Yaw + Spread.X, 0.f);
	const FVector& End = Location + AimRotation.Vector() * CurrentWeapon->GetRange();
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoreActors);
	GetWorld()->LineTraceSingleByChannel(Hit, Location, End, TraceChannel, QueryParams);
	//DrawDebugLine(GetWorld(), Location, Hit.IsValidBlockingHit() ? Hit.Location : End, Hit.IsValidBlockingHit() ? FColor::Green : FColor::Red, false, 5.f);

	// TEST
	if(!Hit.IsValidBlockingHit())
	{
		Hit.Distance = CurrentWeapon->GetRange();
		Hit.Location = End;
	}
}

void UHitscanAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/*if(ActorInfo->IsLocallyControlled())
	{// If locally controlled, play local firing animation
	FGameplayCueParameters Params;
	Params.Instigator = CHARACTER;
	Params.EffectContext = GET_ASC->MakeEffectContextExtended(CHARACTER);
	Params.RawMagnitude = PlayRate;
	GET_ASC->ExecuteGameplayCueLocal(LocalFiringCue, Params);
	}*/

	// Set CurrentWeapon ref if not already set
	if(!CurrentWeapon)
	{
		CurrentWeapon = CURRENTWEAPONTYPE(AHitscanWeapon);
		if(!CurrentWeapon)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
			return;
		}
	}

	// Only fire if not currently firing
	if(GetASC()->HasMatchingGameplayTag(FiringStateTag))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	const EFireMode FireMode = CurrentWeapon->GetFireMode();
	if(FireMode == EFireMode::SemiAuto)
	{
		Hitscan();
		GetASC()->AddLooseGameplayTagForDurationSingle(FiringStateTag, CurrentWeapon->GetRateOfFire());
		
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}
	else if(FireMode == EFireMode::FullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(RateOfFireTimerHandle, this, &UHitscanAbility::Hitscan, CurrentWeapon->GetRateOfFire(), true, 0.f);
		GetASC()->AddLooseGameplayTag(FiringStateTag);
		
		UAbilityTask_WaitInputRelease* Task = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
		Task->OnRelease.AddDynamic(this, &UHitscanAbility::FullAuto_PrimaryFireReleased);
		Task->Activate();
	}
	else if(FireMode == EFireMode::Burst)
	{
		BurstNumFired = 0;
		
		//GetWorld()->GetTimerManager().SetTimer(RateOfFireTimerHandle, this, &UHitscanAbility::Hitscan, CurrentWeapon->GetBurstRateOfFire(), true, 0.f);
		GetWorld()->GetTimerManager().SetTimer(RateOfFireTimerHandle, this, &UHitscanAbility::Burst_Hitscan, CurrentWeapon->GetBurstRateOfFire(), true, 0.f);
		GetASC()->AddLooseGameplayTagForDurationSingle(FiringStateTag, CurrentWeapon->GetBurstRateOfFire() * (CurrentWeapon->GetNumShotsPerBurst() - 1) + CurrentWeapon->GetRateOfFire());
	}
}

void UHitscanAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	BurstNumFired = 0;
	GetWorld()->GetTimerManager().ClearTimer(RateOfFireTimerHandle);
}


void UHitscanAbility::Hitscan()
{
	if(!CanActivateAbility(CurrentSpecHandle, CurrentActorInfo, nullptr, nullptr, nullptr)) return;
	
	TArray<FHitResult> Hits;
	for(int32 i = 0; i < CurrentWeapon->GetNumShots(); i++)
	{
		FHitResult Hit;
		DoLineTrace(Hit, GetCharacter()->GetCamera()->GetComponentLocation(), GetCharacter()->GetBaseAimRotation(), {GetCharacter()});
		Hits.Add(Hit);
	}

	FGameplayAbilityTargetDataHandle TargetData;
	for(const FHitResult& Hit : Hits)
	{
		if(Hit.IsValidBlockingHit())
		{
			TargetData.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit)); 
		}
		else // TEST
		{
			TargetData.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
		}
	}
	
	if(!TargetData.Data.IsEmpty())
	{
		if(!CurrentActorInfo->IsNetAuthority())
		{// Replicate target data if server, otherwise simply call target data received
			const FPredictionKey& Key = CurrentActivationInfo.GetActivationPredictionKey();
			GetASC()->ServerSetReplicatedTargetData(CurrentSpecHandle, Key, TargetData, FGameplayTag(), Key);
			OnFire(TargetData, true);
		}
		else
		{
			Server_ReceivedTargetData(TargetData, FGameplayTag());
		}
	}
	else if(!CurrentActorInfo->IsNetAuthority())
	{// If no valid hits, RPC nothing. If server, simply call event received
		const FPredictionKey& Key = CurrentActivationInfo.GetActivationPredictionKey();
		GetASC()->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericSignalFromClient, CurrentSpecHandle, Key, Key);
		OnFire(TargetData, true);
	}
	else
	{
		Server_ReceivedEvent();
	}
}


void UHitscanAbility::Server_ReceivedEvent_Implementation()
{// If failed prediction, return and update client predicted ammo
	if(!CurrentActorInfo->IsLocallyControlled() && !CanActivateAbility(CurrentSpecHandle, CurrentActorInfo, nullptr, nullptr, nullptr))
	{
		CurrentWeapon->UpdateClientAmmo();
		return;
	}
	
	//GetASC()->NetMulticast_InvokeGameplayCueExecuted(NetMulticastFiringCue, CurrentActivationInfo.GetActivationPredictionKey(), GetASC()->MakeEffectContext());
	OnFire(FGameplayAbilityTargetDataHandle(), CurrentActorInfo->IsLocallyControlled());
}

void UHitscanAbility::Server_ReceivedTargetData_Implementation(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag Tag)
{
	if(!CurrentActorInfo->IsLocallyControlled())
	{
		GetASC()->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
		
		// If failed prediction, return and update client predicted ammo
		if(!CanActivateAbility(CurrentSpecHandle, CurrentActorInfo, nullptr, nullptr, nullptr))
		{
			CurrentWeapon->UpdateClientAmmo();
			return;
		}
	}
	
	const FGameplayEffectContextHandle& Context = GetASC()->MakeEffectContext();
	((FGameplayEffectContextExtended*)Context.Get())->AddTargetData(TargetData);

	const FGameplayEffectSpecHandle& Spec = GetASC()->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	Spec.Data.Get()->DynamicAssetTags.AppendTags(GetInventory()->GetCurrentWeapon()->GetDamageCalculationTags());

	GetASC()->NetMulticast_InvokeGameplayCueExecuted(NetMulticastImpactCue, CurrentActivationInfo.GetActivationPredictionKey(), Context);

	TArray<AActor*> Targets;
	for(const TSharedPtr<FGameplayAbilityTargetData>& Data : GAS::FilterTargetData<FGameplayAbilityTargetData_SingleTargetHit>(TargetData).Data)
		if(const FHitResult* Hit = Data.Get()->GetHitResult())
			if(Targets.Find(Hit->GetActor()) == INDEX_NONE && GAS::ShouldProjectileApplyForce(Hit->GetComponent(), Hit->BoneName))
				Targets.Add(Hit->GetActor());

	for(const AActor* Target : Targets)
	{// For each simulating target, filter target data by the target and net multicast apply knockback
		FGameplayCueParameters Params(FGameplayEffectContextHandle(new FGameplayEffectContextExtended(GetCharacter(), GAS::FilterTargetDataByActor(Target, TargetData))));
		Params.RawMagnitude = IDamageCalculationInterface::Execute_CalculateDamage(GetInventory()->GetCurrentWeapon(), Target, Spec);
		GetASC()->NetMulticast_InvokeGameplayCueExecuted_WithParams(NetMulticastKnockbackCue, FPredictionKey(), Params);
	}

	// Filter by hit actors that implement the IDamageInterface
	TArray<AActor*> DamageableHits;
	for(int32 i = 0; i < TargetData.Data.Num(); i++)
	{
		if(const FHitResult* Hit = TargetData.Data[i].Get()->GetHitResult())
		{
			if(Hit->GetActor() && Hit->GetActor()->GetClass()->ImplementsInterface(UDamageInterface::StaticClass()))
			{
				if(DamageableHits.Find(Hit->GetActor()) == INDEX_NONE)
				{// If has none
					DamageableHits.Add(Hit->GetActor());
				}
			}
		}
	}

	// Apply damage to each damageable target
	if(!DamageableHits.IsEmpty())
	{
		if(Spec.IsValid())
		{
			for(AActor* DamageableHit : DamageableHits)
			{
				IDamageInterface::ApplyDamage(GetASC(), DamageableHit, Spec);
			}
		}
	}

	// Generic received event to decrement ammo and apply firing tag
	//Server_ReceivedEvent_Implementation();
	OnFire(TargetData, CurrentActorInfo->IsLocallyControlled());
}

void UHitscanAbility::OnFire_Implementation(const FGameplayAbilityTargetDataHandle& TargetData, const bool bIsLocal)
{
	CurrentWeapon->DecrementAmmo();
	
	if(bIsLocal)
	{
		CurrentWeapon->OnFireWeapon(TargetData);
		
		if(CurrentActorInfo->IsNetAuthority())
			CurrentWeapon->Multi_OnFireWeapon(TargetData, true);
	}
	else
	{
		CurrentWeapon->Multi_OnFireWeapon(TargetData, true);
	}
}

void UHitscanAbility::FullAuto_PrimaryFireReleased_Implementation(const float Time)
{
	GetASC()->RemoveLooseGameplayTag(FiringStateTag, 10);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

// Called on loop when burst firing, fires until burst num fired exceeds numshots
void UHitscanAbility::Burst_Hitscan_Implementation()
{
	Hitscan();
	
	if(++BurstNumFired >= CurrentWeapon->GetNumShotsPerBurst())
	{
		BurstNumFired = 0;
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	}
}




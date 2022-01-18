// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/HitscanAbility.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/Effects/DamageEffect.h"

UHitscanAbility::UHitscanAbility()
{
	//LineTraceObject = CreateDefaultSubobject<ULineTraceObject>(TEXT("LineTraceObject"));
	DamageEffectClass = UDamageEffect::StaticClass();

	FiringStateTag = TAG("WeaponState.Firing");
	LocalFiringCue = TAG("GameplayCue.FireWeapon.Local");
	NetMulticastFiringCue = TAG("GameplayCue.FireWeapon.NetMulticast");
	NetMulticastKnockbackCue = TAG("GameplayCue.Knockback");
	NetMulticastImpactCue = TAG("GameplayCue.Impact.Bullet");
	
	Input = EAbilityInput::PrimaryFire;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Equipping"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Reloading"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Firing"));
}

void UHitscanAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if(ActorInfo->IsNetAuthority() && !ActorInfo->IsLocallyControlled())
	{// If server and not locally controlled, setup RPC delegates
		GET_ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericSignalFromClient, Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UHitscanAbility::Server_ReceivedEvent);
		GET_ASC->AbilityTargetDataSetDelegate(Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UHitscanAbility::Server_ReceivedTargetData);
	}
}

bool UHitscanAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{// Must be able to fire to activate ability
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && GetInventory()->GetCurrentWeapon() && GetInventory()->GetCurrentWeapon()->CanFire();
}

void UHitscanAbility::DoLineTrace_Implementation(FHitResult& Hit, const FVector& Location, const FRotator& Rotation, const TArray<AActor*>& IgnoreActors)
{
	const FVector2D& CalcSpread = FVector2D(FMath::FRandRange(-Spread.Y, Spread.Y), FMath::FRandRange(-Spread.X, Spread.X)) * CalculateSpreadMagnitude();
	const FRotator& RandRotation = { Rotation.Pitch + CalcSpread.Y, Rotation.Yaw + CalcSpread.X, 0.f };
	const FVector& End = Location + RandRotation.Vector() * Range;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoreActors);
	GetWorld()->LineTraceSingleByChannel(Hit, Location, End, TraceChannel, QueryParams);
	//DrawDebugLine(GetWorld(), Location, Hit.IsValidBlockingHit() ? Hit.Location : End, Hit.IsValidBlockingHit() ? FColor::Green : FColor::Red, false, 5.f);
}

void UHitscanAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AWeapon* CurrentWeapon = CURRENTWEAPON;

	if(ActorInfo->IsLocallyControlled())
	{// If locally controlled, play local firing animation
		FGameplayCueParameters Params;
		Params.Instigator = CHARACTER;
		Params.EffectContext = GET_ASC->MakeEffectContextExtended(CHARACTER);
		Params.RawMagnitude = PlayRate;
		GET_ASC->ExecuteGameplayCueLocal(LocalFiringCue, Params);
	}

	TArray<FHitResult> Hits;
	for(int32 i = 0; i < CurrentWeapon->GetNumShots(); i++)
	{
		FHitResult Hit;
		DoLineTrace(Hit, GetCharacter()->GetCamera()->GetComponentLocation(), GetCharacter()->GetCamera()->GetComponentRotation(), {GetCharacter()});
		Hits.Add(Hit);
	}

	FGameplayAbilityTargetDataHandle DataHandle;
	for(const FHitResult& Hit : Hits)
	{
		if(Hit.IsValidBlockingHit())
			DataHandle.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit)); 
	}
		
	if(!DataHandle.Data.IsEmpty())
	{
		if(!ActorInfo->IsNetAuthority())
		{// Replicate target data if server, otherwise simply call target data received
			const FPredictionKey& Key = ActivationInfo.GetActivationPredictionKey();
			GetASC()->ServerSetReplicatedTargetData(Handle, Key, DataHandle, FGameplayTag(), Key);
			OnFire(CurrentWeapon);
		}
		else
		{
			Server_ReceivedTargetData(DataHandle, FGameplayTag());
		}
	}
	else if(!ActorInfo->IsNetAuthority())
	{// If no valid hits, RPC nothing. If server, simply call event received
		const FPredictionKey& Key = ActivationInfo.GetActivationPredictionKey();
		GetASC()->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericSignalFromClient, Handle, Key, Key);
		OnFire(CurrentWeapon);
	}
	else
	{
		Server_ReceivedEvent();
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UHitscanAbility::Server_ReceivedEvent_Implementation() const
{// If failed prediction, return and update client predicted ammo
	AWeapon* CurrentWeapon = GetInventory()->GetCurrentWeapon();
	if(!CurrentActorInfo->IsLocallyControlled() && !CanActivateAbility(CurrentSpecHandle, CurrentActorInfo, nullptr, nullptr, nullptr))
	{
		CurrentWeapon->UpdateClientAmmo();
		return;
	}
	
	GetASC()->AddLooseGameplayTagForDurationSingle(FiringStateTag, CurrentWeapon->GetRateOfFire());
	OnFire(CurrentWeapon);
}

void UHitscanAbility::Server_ReceivedTargetData_Implementation(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag) const
{
	if(!CurrentActorInfo->IsLocallyControlled())
	{
		GetASC()->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		// If failed prediction, return and update client predicted ammo
		if(!CanActivateAbility(CurrentSpecHandle, CurrentActorInfo, nullptr, nullptr, nullptr))
		{
			GetInventory()->GetCurrentWeapon()->UpdateClientAmmo();
			return;
		}
	}
	
	const FGameplayEffectContextHandle& Context = GetASC()->MakeEffectContext();
	((FGameplayEffectContextExtended*)Context.Get())->AddTargetData(Handle);

	const FGameplayEffectSpecHandle& Spec = GetASC()->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	Spec.Data.Get()->DynamicAssetTags.AppendTags(GetInventory()->GetCurrentWeapon()->GetDamageCalculationTags());

	GetASC()->NetMulticast_InvokeGameplayCueExecuted(NetMulticastImpactCue, CurrentActivationInfo.GetActivationPredictionKey(), Context);
	GetASC()->NetMulticast_InvokeGameplayCueExecuted(NetMulticastFiringCue, CurrentActivationInfo.GetActivationPredictionKey(), GetASC()->MakeEffectContext());

	TArray<AActor*> Targets;
	for(const TSharedPtr<FGameplayAbilityTargetData>& Data : GAS::FilterTargetData<FGameplayAbilityTargetData_SingleTargetHit>(Handle).Data)
		if(const FHitResult* Hit = Data.Get()->GetHitResult())
			if(Targets.Find(Hit->GetActor()) == INDEX_NONE && GAS::ShouldProjectileApplyForce(Hit->GetComponent(), Hit->BoneName))
				Targets.Add(Hit->GetActor());

	for(const AActor* Target : Targets)
	{// For each simulating target, filter target data by the target and net multicast apply knockback
		FGameplayCueParameters Params(FGameplayEffectContextHandle(new FGameplayEffectContextExtended(GetCharacter(), GAS::FilterTargetDataByActor(Target, Handle))));
		Params.RawMagnitude = IDamageCalculationInterface::Execute_CalculateDamage(GetInventory()->GetCurrentWeapon(), Target, Spec);
		GetASC()->NetMulticast_InvokeGameplayCueExecuted_WithParams(NetMulticastKnockbackCue, FPredictionKey(), Params);
	}

	// Filter by hit actors that implement the IDamageInterface
	TArray<AActor*> DamageableHits;
	for(int32 i = 0; i < Handle.Data.Num(); i++)
	{
		if(const FHitResult* Hit = Handle.Data[i].Get()->GetHitResult())
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
	Server_ReceivedEvent_Implementation();
}

void UHitscanAbility::OnFire_Implementation(AWeapon* CurrentWeapon) const
{
	CurrentWeapon->DecrementAmmo();
	GetASC()->AddLooseGameplayTagForDurationSingle(FiringStateTag, CurrentWeapon->GetRateOfFire());
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/HitscanAbility.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/Effects/DamageEffect.h"
#include "Objects/LineTraceObject.h"

UHitscanAbility::UHitscanAbility()
{
	LineTraceObject = CreateDefaultSubobject<ULineTraceObject>(TEXT("LineTraceObject"));
	DamageEffectClass = UDamageEffect::StaticClass();
	
	Input = EAbilityInput::PrimaryFire;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Stunned"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Equipping"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Reloading"));
}

void UHitscanAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if(const AWeapon* Weapon = INVENTORY->GetCurrent())
	{
		LineTraceObject.Get()->Range = Weapon->GetRange();
		NumShots = Weapon->GetNumShots();
	}

	if(ActorInfo->IsNetAuthority() && !ActorInfo->IsLocallyControlled())
	{
		GET_ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericSignalFromClient, Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UHitscanAbility::Server_ReceivedEvent);
		GET_ASC->AbilityTargetDataSetDelegate(Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UHitscanAbility::Server_ReceivedTargetData);
	}
}

bool UHitscanAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{// Must be able to fire to activate ability
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && GetInventory()->GetCurrent() && GetInventory()->GetCurrent()->CanFire();
}


void UHitscanAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if(ActorInfo->IsLocallyControlled())
	{// If locally controlled, play local firing animation
		FGameplayCueParameters Params;
		Params.Instigator = CHARACTER;
		Params.EffectContext = GET_ASC->MakeEffectContextExtended(CHARACTER);
		GET_ASC->ExecuteGameplayCueLocal(TAG("GameplayCue.FireWeapon.Local"), Params);
	}

	TArray<FHitResult> Hits;
	for(int32 i = 0; i < NumShots; i++)
	{
		Hits.Add(LineTraceObject->DoLineTrace(GetCharacter()->GetCamera()->GetComponentLocation(), GetCharacter()->GetCamera()->GetComponentRotation(), {GetCharacter()}, 1.f, true));
	}

	FGameplayAbilityTargetDataHandle DataHandle;
	for(const FHitResult& Hit : Hits)
	{
		if(Hit.IsValidBlockingHit())
			DataHandle.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
	}
		
	if(!DataHandle.Data.IsEmpty())
	{
		// Replicate target data if server, otherwise simply call the function
		if(!ActorInfo->IsNetAuthority())
		{
			const FPredictionKey& Key = ActivationInfo.GetActivationPredictionKey();
			GetASC()->ServerSetReplicatedTargetData(Handle, Key, DataHandle, FGameplayTag(), Key);
			GetInventory()->GetCurrent()->OnFire();
		}
		else
		{
			Server_ReceivedTargetData(DataHandle, FGameplayTag());
		}
	}
	else if(!ActorInfo->IsNetAuthority())
	{
		const FPredictionKey& Key = ActivationInfo.GetActivationPredictionKey();
		GetASC()->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericSignalFromClient, Handle, Key, Key);
		GetInventory()->GetCurrent()->OnFire();
	}
	else
	{
		Server_ReceivedEvent();
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UHitscanAbility::Server_ReceivedEvent_Implementation() const
{
	if(!CanActivateAbility(CurrentSpecHandle, CurrentActorInfo, nullptr, nullptr, nullptr))
	{
		GetCharacter()->GetCurrentWeapon()->UpdateClientAmmo();
		return;
	}
	
	GetInventory()->GetCurrent()->OnFire();
}

void UHitscanAbility::Server_ReceivedTargetData_Implementation(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag) const
{
	GetASC()->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

	// if can't activate ability server-side. Do nothing
	if(!CanActivateAbility(CurrentSpecHandle, CurrentActorInfo, nullptr, nullptr, nullptr))
	{
		GetCharacter()->GetCurrentWeapon()->UpdateClientAmmo();
		return;
	}
	
	const FGameplayEffectContextHandle& Context = GetASC()->MakeEffectContext();
	((FGameplayEffectContextExtended*)Context.Get())->AddTargetData(Handle);

	const FGameplayEffectSpecHandle& Spec = GetASC()->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	Spec.Data.Get()->DynamicAssetTags.AppendTags(GetInventory()->GetCurrent()->GetDamageCalculationTags());

	GetASC()->NetMulticast_InvokeGameplayCueExecuted(TAG("GameplayCue.Impact.Bullet"), CurrentActivationInfo.GetActivationPredictionKey(), Context);
	GetASC()->NetMulticast_InvokeGameplayCueExecuted(TAG("GameplayCue.FireWeapon.NetMulticast"), CurrentActivationInfo.GetActivationPredictionKey(), GetASC()->MakeEffectContext());

	TArray<AActor*> Targets;
	for(const TSharedPtr<FGameplayAbilityTargetData>& Data : GAS::FilterTargetData<FGameplayAbilityTargetData_SingleTargetHit>(Handle).Data)
		if(const FHitResult* Hit = Data.Get()->GetHitResult())
			if(Hit->Component.IsValid() && Hit->GetComponent()->IsSimulatingPhysics(Hit->BoneName) && Targets.Find(Hit->GetActor()) == INDEX_NONE)
				Targets.Add(Hit->GetActor());

	for(const AActor* Target : Targets)
	{
		FGameplayCueParameters Params(FGameplayEffectContextHandle(new FGameplayEffectContextExtended(GetCharacter(), GAS::FilterTargetDataByActor(Target, Handle))));
		Params.RawMagnitude = IDamageCalculationInterface::Execute_CalculateDamage(GetInventory()->GetCurrent(), Target, Spec);
		GetASC()->NetMulticast_InvokeGameplayCueExecuted_WithParams(TAG("GameplayCue.Knockback"), FPredictionKey(), Params);
	}
	
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

	Server_ReceivedEvent_Implementation();
}

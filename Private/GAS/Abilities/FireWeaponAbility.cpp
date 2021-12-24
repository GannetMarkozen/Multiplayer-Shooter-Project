// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/FireWeaponAbility.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/Effects/DamageEffect.h"
#include "Objects/LineTraceObject.h"

UFireWeaponAbility::UFireWeaponAbility()
{
	LineTraceObject = CreateDefaultSubobject<ULineTraceObject>(TEXT("LineTraceObject"));
	DamageEffectClass = UDamageEffect::StaticClass();
	
	Input = EAbilityInput::PrimaryFire;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Stunned"));
}

void UFireWeaponAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if(ActorInfo->IsNetAuthority() && !ActorInfo->IsLocallyControlled())
	{
		GET_ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericSignalFromClient, Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UFireWeaponAbility::Server_ReceivedEvent);
		GET_ASC->AbilityTargetDataSetDelegate(Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UFireWeaponAbility::Server_ReceivedTargetData);
	}
}

bool UFireWeaponAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{// Must be able to fire to activate ability
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && GetInventory()->GetCurrent() && GetInventory()->GetCurrent()->CanFire();
}


void UFireWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	TArray<FHitResult> Hits;
	for(int32 i = 0; i < NumShots; i++)
	{
		Hits.Add(LineTraceObject->DoLineTrace(GetCharacter()->GetCamera()->GetComponentLocation(), GetCharacter()->GetCamera()->GetComponentRotation(), {GetCharacter()}, 1.f, true));
	}

	FGameplayAbilityTargetDataHandle DataHandle;
	for(const FHitResult& Hit : Hits)
	{
		if(!Hit.IsValidBlockingHit()) continue;
		FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
		DataHandle.Add(Data);
	}
		
	if(!DataHandle.Data.IsEmpty())
	{
		//const FGameplayAbilityTargetDataHandle& DataHandle(Data);

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

void UFireWeaponAbility::Server_ReceivedEvent_Implementation() const
{
	//PRINT(TEXT("%s: Received generic replicated event"), *AUTHTOSTRING(CurrentActorInfo->IsNetAuthority()));
	GetInventory()->GetCurrent()->OnFire();
}

void UFireWeaponAbility::Server_ReceivedTargetData_Implementation(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag) const
{
	//TArray<IDamageInterface*> DamageableHits;
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
		const FGameplayEffectContextHandle& Context = GetASC()->MakeEffectContext();
		((FGameplayEffectContextExtended*)Context.Get())->AddTargetData(Handle);

		const FGameplayEffectSpecHandle& Spec = GetASC()->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
		Spec.Data.Get()->DynamicAssetTags.AppendTags(GetInventory()->GetCurrent()->GetDamageCalculationTags());
		Spec.Data.Get()->SetByCallerTagMagnitudes.Add(UAbilitySystemGlobalsExtended::Get().GetBaseDamageTag(), GetInventory()->GetCurrent()->GetBaseDamage());
		
		if(Spec.IsValid())
		{
			for(AActor* DamageableHit : DamageableHits)
			{
				IDamageInterface::ApplyDamage(GetASC(), DamageableHit, Spec);
			}
		}
	}

	GetASC()->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
	
	Server_ReceivedEvent();
}

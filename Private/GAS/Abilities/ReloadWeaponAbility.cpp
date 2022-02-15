// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/ReloadWeaponAbility.h"

#include "Character/CharacterInventoryComponent.h"
#include "Character/ShooterCharacter.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "GAS/Abilities/AbilityTasks/AbilityTask_WaitMontageCompleted.h"
#include "GAS/Abilities/Weapons/ProjectileWeapon.h"


UReloadWeaponAbility::UReloadWeaponAbility()
{
	Input = EAbilityInput::Reload;

	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Reloading"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Interacting"));
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UReloadWeaponAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	CurrentWeapon = CURRENTWEAPONTYPE(AProjectileWeapon);
	if(bReloadOnEnd && ActorInfo->IsNetAuthority())
	{
		if(CurrentWeapon->GetAmmo() <= 0)
			GET_ASC->TryActivateAbility(Spec.Handle);
		
		CurrentWeapon->AmmoDelegate.AddDynamic(this, &UReloadWeaponAbility::AmmoChanged);
		
		for(TArray<FGameplayTag>::TConstIterator Itr(TAG_CONTAINER({ActivationRequiredTags, ActivationBlockedTags}).CreateConstIterator()); Itr; ++Itr)
			GET_ASC->RegisterGameplayTagEvent(*Itr, EGameplayTagEventType::AnyCountChange).AddUObject(this, &UReloadWeaponAbility::TagsChanged);
	}
}

void UReloadWeaponAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if(bReloadOnEnd && ActorInfo->IsNetAuthority())
	{
		CurrentWeapon->AmmoDelegate.RemoveAll(this);
		for(TArray<FGameplayTag>::TConstIterator Itr(TAG_CONTAINER({ActivationRequiredTags, ActivationBlockedTags}).CreateConstIterator()); Itr; ++Itr)
			GET_ASC->RegisterGameplayTagEvent(*Itr, EGameplayTagEventType::AnyCountChange).RemoveAll(this);
	}
}

void UReloadWeaponAbility::AmmoChanged_Implementation(const int32 Ammo)
{
	if(Ammo <= 0)
		GetASC()->TryActivateAbility(CurrentSpecHandle);
}

void UReloadWeaponAbility::TagsChanged_Implementation(FGameplayTag Tag, int32 Count)
{
	if(CurrentWeapon->GetAmmo() <= 0)
		GetASC()->TryActivateAbility(CurrentSpecHandle);
}



bool UReloadWeaponAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const AProjectileWeapon* Current = CURRENTWEAPONTYPE(AProjectileWeapon);
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && Current && Current->GetAmmo() < static_cast<const AProjectileWeapon*>(Current->GetClass()->GetDefaultObject())->GetAmmo() && Current->GetReserveAmmo() > 0;
}


void UReloadWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CurrentWeapon = CURRENTWEAPONTYPE(AProjectileWeapon);
	if(HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{// Init gameplay cue params
		FGameplayCueParameters Params;
		Params.Instigator = CHARACTER;
		Params.EffectContext = GET_ASC->MakeEffectContext();
		Params.RawMagnitude = PlayRate;
		
		// Add reloading tags on both server and client
		if(ActorInfo->IsNetAuthority())
		{// Play third person reload animation on all instances
			if(CurrentWeapon->GetEquipMontage())
			{
				GET_ASC->NetMulticast_InvokeGameplayCueExecuted_WithParams(NetMulticastReloadingCue, ActivationInfo.GetActivationPredictionKey(), Params);
				PRINT(TEXT("%s: Playing Equip Montage"), *AUTHTOSTRING(ActorInfo->IsNetAuthority()));
			}
				

			// If server, add reload state and at the end call the callback delegate that sets the ammo
			TDelegate<void(UGASAbilitySystemComponent*, const FGameplayTag&)> CallbackDelegate;
			CallbackDelegate.BindUObject(this, &UReloadWeaponAbility::Server_SetAmmo);
			GET_ASC->AddLooseGameplayTagForDurationSingle_Static(ReloadStateTag, CurrentWeapon->GetReloadDuration() / PlayRate, &CallbackDelegate);
		}
		else
		{// If client, add reload state tag but do not set ammo
			GET_ASC->AddLooseGameplayTagForDuration(ReloadStateTag, CurrentWeapon->GetReloadDuration() / PlayRate);
		}
		
		if(ActorInfo->IsLocallyControlled())
		{// Play first person reload animation locally
			GET_ASC->ExecuteGameplayCueLocal(LocalReloadingCue, Params);
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}
	else EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UReloadWeaponAbility::Server_SetAmmo(UGASAbilitySystemComponent* ASC, const FGameplayTag& Tag)
{
	//AWeapon* CurrentWeapon = ((FGameplayAbilityActorInfoExtended*)ASC->AbilityActorInfo.Get())->Character.Get()->GetCurrentWeapon();
	const int32 ReserveAmmo = CurrentWeapon->GetReserveAmmo();
	const int32 OldAmmo = CurrentWeapon->GetAmmo();
	CurrentWeapon->SetAmmo(FMath::Min<int32>(ReserveAmmo + OldAmmo, static_cast<const AProjectileWeapon*>(CurrentWeapon->GetClass()->GetDefaultObject())->GetAmmo()));
	CurrentWeapon->SetReserveAmmo(ReserveAmmo + OldAmmo - CurrentWeapon->GetAmmo());
}


void UReloadWeaponAbility::Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	Super::Client_PredictionFailed_Implementation(ActorInfo);
	
	if(UAnimInstance* AnimInstance = ActorInfo.Character.Get()->GetMesh()->GetAnimInstance())
		AnimInstance->Montage_Stop(0.f, CurrentWeapon ? CurrentWeapon->GetReloadMontage() : nullptr);
}

void UReloadWeaponAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if(bWasCancelled && !ActorInfo->IsNetAuthority())
		if(UAnimInstance* AnimInstance = CHARACTER->GetMesh()->GetAnimInstance())
			AnimInstance->Montage_Stop(0.f, CurrentWeapon ? CurrentWeapon->GetReloadMontage() : nullptr);
}


/*
 *
 *
 *
 *
 *
 */












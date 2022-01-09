// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/ReloadWeaponAbility.h"

#include "Character/CharacterInventoryComponent.h"
#include "Character/ShooterCharacter.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "GAS/Abilities/AbilityTasks/AbilityTask_WaitMontageCompleted.h"


UReloadWeaponAbility::UReloadWeaponAbility()
{
	Input = EAbilityInput::Reload;

	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Reloading"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Interacting"));
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UReloadWeaponAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if(bReloadOnEnd && ActorInfo->IsNetAuthority())
		GET_ASC->GiveAbility(FGameplayAbilitySpec(UReloadWeaponActivationAbility::StaticClass(), 1.f, INDEX_NONE, this));
}

void UReloadWeaponAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if(bReloadOnEnd && ActorInfo->IsNetAuthority())
		if(const FGameplayAbilitySpec* SpecHandle = GET_ASC->FindAbilitySpecFromClass(UReloadWeaponActivationAbility::StaticClass()))
			GET_ASC->SetRemoveAbilityOnEnd(SpecHandle->Handle);
}



bool UReloadWeaponAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const AWeapon* Current = CHARACTER->GetCurrentWeapon();
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && Current && Current->GetAmmo() < CastChecked<AWeapon>(Current->GetClass()->GetDefaultObject())->GetAmmo() && Current->GetReserveAmmo() > 0;
}


void UReloadWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if(HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{// Init gameplay cue params
		FGameplayCueParameters Params;
		Params.Instigator = CHARACTER;
		Params.EffectContext = GET_ASC->MakeEffectContext();
		Params.RawMagnitude = PlayRate;
		
		// Add reloading tags on both server and client
		if(ActorInfo->IsNetAuthority())
		{// Play third person reload animation on all instances
			if(CHARACTER->GetCurrentWeapon()->GetTP_EquipMontage())
				GET_ASC->NetMulticast_InvokeGameplayCueExecuted_WithParams(NetMulticastReloadingCue, ActivationInfo.GetActivationPredictionKey(), Params);

			// If server, add reload state and at the end call the callback delegate that sets the ammo
			TDelegate<void(UGASAbilitySystemComponent*, const FGameplayTag&)> CallbackDelegate;
			CallbackDelegate.BindUObject(this, &UReloadWeaponAbility::Server_SetAmmo);
			GET_ASC->AddLooseGameplayTagForDurationSingle_Static(ReloadStateTag, CHARACTER->GetCurrentWeapon()->GetReloadDuration() / PlayRate, &CallbackDelegate);
		}
		else
		{// If client, add reload state tag but do not set ammo
			GET_ASC->AddLooseGameplayTagForDuration(ReloadStateTag, CHARACTER->GetCurrentWeapon()->GetReloadDuration() / PlayRate);
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
	const AShooterCharacter* Character = ((FGameplayAbilityActorInfoExtended*)ASC->AbilityActorInfo.Get())->Character.Get();
	const int32 ReserveAmmo = Character->GetCurrentWeapon()->GetReserveAmmo();
	const int32 OldAmmo = Character->GetCurrentWeapon()->GetAmmo();
	Character->GetCurrentWeapon()->SetAmmo(FMath::Min<int32>(ReserveAmmo, static_cast<const AWeapon*>(Character->GetCurrentWeapon()->GetClass()->GetDefaultObject())->GetAmmo()));
	Character->GetCurrentWeapon()->SetReserveAmmo(ReserveAmmo + OldAmmo - Character->GetCurrentWeapon()->GetAmmo());
}


void UReloadWeaponAbility::Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	Super::Client_PredictionFailed_Implementation(ActorInfo);
	
	if(UAnimInstance* AnimInstance = ActorInfo.Character.Get()->GetFP_Mesh()->GetAnimInstance())
		AnimInstance->Montage_Stop(0.f, ActorInfo.Inventory.Get()->GetCurrent() ? ActorInfo.Inventory.Get()->GetCurrent()->GetFP_ReloadMontage() : nullptr);
}

void UReloadWeaponAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if(bWasCancelled && !ActorInfo->IsNetAuthority())
		if(UAnimInstance* AnimInstance = CHARACTER->GetFP_Mesh()->GetAnimInstance())
			AnimInstance->Montage_Stop(0.f, INVENTORY->GetCurrent() ? INVENTORY->GetCurrent()->GetFP_ReloadMontage() : nullptr);
}


/*
 *
 *
 *
 *
 *
 */

UReloadWeaponActivationAbility::UReloadWeaponActivationAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UReloadWeaponActivationAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if(ActorInfo->IsNetAuthority() && CHARACTER->GetCurrentWeapon() /* Sanity check */)
		CHARACTER->GetCurrentWeapon()->AmmoDelegate.AddDynamic(this, &UReloadWeaponActivationAbility::AmmoChanged);
}

void UReloadWeaponActivationAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if(ActorInfo->IsNetAuthority() && CHARACTER->GetCurrentWeapon())
		CHARACTER->GetCurrentWeapon()->AmmoDelegate.RemoveAll(this);
}

void UReloadWeaponActivationAbility::AmmoChanged(int32 Ammo)
{
	if(Ammo <= 0.f && GetCharacter()->GetCurrentWeapon() && GetCharacter()->GetCurrentWeapon()->GetReserveAmmo() > 0)
		GetASC()->TryActivateAbilityByClass(UReloadWeaponAbility::StaticClass());
}













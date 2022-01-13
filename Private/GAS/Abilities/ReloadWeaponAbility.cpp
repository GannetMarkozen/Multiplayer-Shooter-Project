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
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UReloadWeaponAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	CurrentWeapon = CURRENTWEAPON;
	if(bReloadOnEnd && ActorInfo->IsNetAuthority())
		CurrentWeapon->AmmoDelegate.AddDynamic(this, &UReloadWeaponAbility::AmmoChanged);
}

void UReloadWeaponAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if(bReloadOnEnd && ActorInfo->IsNetAuthority())
		CurrentWeapon->AmmoDelegate.RemoveAll(this);
}

void UReloadWeaponAbility::AmmoChanged_Implementation(const int32 Ammo)
{
	if(Ammo <= 0)
		GetASC()->TryActivateAbility(CurrentSpecHandle);
}


bool UReloadWeaponAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const AWeapon* Current = CURRENTWEAPON;
	if(ActorInfo->IsNetAuthority())
	{
		if(!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) {PRINT(TEXT("Can Activate Ability"));}
		else if(!Current) {PRINT(TEXT("Current"));}
		else if(!(Current->GetAmmo() < CastChecked<AWeapon>(Current->GetClass()->GetDefaultObject())->GetAmmo())) {PRINT(TEXT("Ammo == %i"), Current->GetAmmo());}
		else if(!(Current->GetReserveAmmo() > 0)) {PRINT(TEXT("Reserve Ammo"));}
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && Current && Current->GetAmmo() < CastChecked<AWeapon>(Current->GetClass()->GetDefaultObject())->GetAmmo() && Current->GetReserveAmmo() > 0;
}


void UReloadWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CurrentWeapon = CURRENTWEAPON;
	if(HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{// Init gameplay cue params
		FGameplayCueParameters Params;
		Params.Instigator = CHARACTER;
		Params.EffectContext = GET_ASC->MakeEffectContext();
		Params.RawMagnitude = PlayRate;
		
		// Add reloading tags on both server and client
		if(ActorInfo->IsNetAuthority())
		{// Play third person reload animation on all instances
			if(CurrentWeapon->GetTP_EquipMontage())
				GET_ASC->NetMulticast_InvokeGameplayCueExecuted_WithParams(NetMulticastReloadingCue, ActivationInfo.GetActivationPredictionKey(), Params);

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
	CurrentWeapon->SetAmmo(FMath::Min<int32>(ReserveAmmo + OldAmmo, static_cast<const AWeapon*>(CurrentWeapon->GetClass()->GetDefaultObject())->GetAmmo()));
	CurrentWeapon->SetReserveAmmo(ReserveAmmo + OldAmmo - CurrentWeapon->GetAmmo());
}


void UReloadWeaponAbility::Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	Super::Client_PredictionFailed_Implementation(ActorInfo);
	
	if(UAnimInstance* AnimInstance = ActorInfo.Character.Get()->GetFP_Mesh()->GetAnimInstance())
		AnimInstance->Montage_Stop(0.f, CurrentWeapon ? CurrentWeapon->GetFP_ReloadMontage() : nullptr);
}

void UReloadWeaponAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if(bWasCancelled && !ActorInfo->IsNetAuthority())
		if(UAnimInstance* AnimInstance = CHARACTER->GetFP_Mesh()->GetAnimInstance())
			AnimInstance->Montage_Stop(0.f, CurrentWeapon ? CurrentWeapon->GetFP_ReloadMontage() : nullptr);
}


/*
 *
 *
 *
 *
 *
 */












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
	ActivationBlockedTags.AddTag(TAG("Status.State.Stunned"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Reloading"));
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UReloadWeaponAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const AWeapon* Current = CHARACTER->GetCurrentWeapon();
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) && Current && Current->Ammo < CastChecked<AWeapon>(Current->GetClass()->GetDefaultObject())->Ammo && Current->ReserveAmmo > 0;
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
		GET_ASC->AddLooseGameplayTagForDuration(TAG("WeaponState.Reloading"), CHARACTER->GetCurrentWeapon()->GetReloadDuration() / PlayRate);
		
		if(ActorInfo->IsNetAuthority())
		{// Play third person reload animation on all instances
			if(CHARACTER->GetCurrentWeapon()->GetTP_EquipMontage())
				GET_ASC->NetMulticast_InvokeGameplayCueExecuted_WithParams(TAG("GameplayCue.Reload.NetMulticast"), ActivationInfo.GetActivationPredictionKey(), Params);

			int32& Ammo = CHARACTER->GetCurrentWeapon()->Ammo;
			int32& ReserveAmmo = CHARACTER->GetCurrentWeapon()->ReserveAmmo;

			const int32 OldAmmo = Ammo;
			Ammo = FMath::Min<int32>(ReserveAmmo, static_cast<AWeapon*>(INVENTORY->GetCurrent()->GetClass()->GetDefaultObject())->Ammo);
			ReserveAmmo += OldAmmo - Ammo;
		}
		if(ActorInfo->IsLocallyControlled())
		{// Play first person reload animation locally
			GET_ASC->ExecuteGameplayCueLocal(TAG("GameplayCue.Reload.Local"), Params);
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}
	else EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
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



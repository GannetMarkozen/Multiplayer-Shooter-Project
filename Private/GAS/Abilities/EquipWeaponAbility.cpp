// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/EquipWeaponAbility.h"

#include "AbilitySystemComponent.h"
#include "Character/InventoryComponent.h"
#include "Character/ShooterCharacter.h"
#include "GameplayAbilities/Public/GameplayModMagnitudeCalculation.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "GAS/Abilities/AbilityTasks/AbilityTask_WaitMontageCompleted.h"

UEquipWeaponAbility::UEquipWeaponAbility()
{
	AbilityTags.AddTag(TAG("Ability.EquipWeapon"));
	
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));

	ActivationOwnedTags.AddTag(TAG("Status.State.Equipping"));
	
	// Activate ability on Status.Equip gameplay event
	FAbilityTriggerData Trigger;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	Trigger.TriggerTag = TAG("Event.Equip");
	AbilityTriggers.Add(Trigger);
}

bool UEquipWeaponAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UEquipWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if(TriggerEventData && GetInventory()->GetWeapons().IsValidIndex(static_cast<int32>(TriggerEventData->EventMagnitude)))
	{
		Equip(static_cast<int32>(TriggerEventData->EventMagnitude));
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}
	else
	{// Cancelled
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UEquipWeaponAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if(bWasCancelled && !ActorInfo->IsNetAuthority())
	{
		Equip(GetInventory()->GetLastIndex(), true);
	}
}

void UEquipWeaponAbility::Equip(const int32 NewIndex, bool bCancelled)
{
	// Players current index. Must be known to re-equip last index if fail
	const int32 CurrentIndex = GetInventory()->GetCurrentIndex();
	
	if(CurrentIndex == NewIndex && !bCancelled) return;
	if(const AWeapon* Weapon = GetInventory()->GetWeapons()[NewIndex])
	{
		if(CurrentActorInfo->IsNetAuthority())
		{
			// Remove last index weapon abilities if valid
			GetInventory()->RemoveAbilities();
                        
			// Give all weapon abilities of current index to owner
			GetInventory()->GiveAbilities(Weapon);
		}

		if(CurrentActorInfo->IsLocallyControlled() && !bCancelled)
		{
			UAbilityTask_WaitMontageCompleted* Task = UAbilityTask_WaitMontageCompleted::WaitMontageCompleted(this, GetCharacter()->GetFP_Mesh()->GetAnimInstance(), Weapon->GetFP_EquipMontage());
			Task->Activate();
			//if(Weapon->GetFP_EquipMontage()) if(UAnimInstance* AnimInstance = ShooterCharacter->GetFP_Mesh()->GetAnimInstance()) AnimInstance->Montage_Play(Weapon->GetFP_EquipMontage());
		}
		// Set the equipped item mesh to the weapon mesh
		GetCharacter()->SetItemMesh(Weapon->GetMesh());

		GetInventory()->SetCurrentIndex(NewIndex);
	}
}

void UEquipWeaponAbility::Client_PredictionFailed_Implementation()
{
	Super::Client_PredictionFailed_Implementation();
	
	Equip(GetInventory()->GetLastIndex(), true);
}


bool UEquipWeaponAbility::EquipWeaponEvent(UAbilitySystemComponent* TargetASC, const int32 Index)
{
	if(!TargetASC) return false;

	FGameplayEventData Payload;
	Payload.EventTag = TAG("Event.Equip");
	Payload.Instigator = TargetASC->GetOwner();
	Payload.Target = TargetASC->GetOwner();
	Payload.EventMagnitude = static_cast<float>(Index);

	return TargetASC->HandleGameplayEvent(TAG("Event.Equip"), &Payload) > 0;
}

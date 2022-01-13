// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/EquipAbility.h"

#include "Character/ShooterCharacter.h"
#include "GAS/Abilities/AbilityTasks/AbilityTask_WaitMontageCompleted.h"

UEquipAbility::UEquipAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));

	FAbilityTriggerData Trigger;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	Trigger.TriggerTag = TAG("Event.Equip");
	AbilityTriggers.Add(Trigger);
}

bool UEquipAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UEquipAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if(TriggerEventData && ActorInfo)
	{
		const int32 Index = TriggerEventData->EventMagnitude;
		PRINT(TEXT("%s: Equipping %i"), *AUTHTOSTRING(ActorInfo->IsNetAuthority()), Index);
		if(INVENTORY->GetWeapons().IsValidIndex(Index))
		{// If valid index, equip
			//INVENTORY->SetCurrentWeapon(Index);
			AWeapon* NewWeapon = INVENTORY->GetWeapons()[Index];
			if(NewWeapon) // Add equipping loose gameplay tags
			{
				if(NewWeapon == INVENTORY->GetCurrentWeapon())
				{// If attempting to equip the current weapon, return
					EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
					return;
				}
				GET_ASC->AddLooseGameplayTagForDurationSingle(TAG("Status.State.Equipping"), NewWeapon->GetWeaponSwapDuration());
			}
			
			Equip(Index, *(FGameplayAbilityActorInfoExtended*)ActorInfo);
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
		else
		{// If invalid index, equip nothing
			Equip(INDEX_NONE, *(FGameplayAbilityActorInfoExtended*)ActorInfo);
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
	}
	else EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UEquipAbility::Equip_Implementation(const int32 Index, const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	// Remove any tags related to weapon state
	ActorInfo.ASC.Get()->RemoveLooseGameplayTagChildren(TAG("WeaponState"));

	// Set current weapon
	ActorInfo.Inventory.Get()->SetCurrentWeapon(Index);
	/*
	if(OldWeapon)
	{
		OldWeapon->GetFP_Mesh()->SetVisibility(false);

		// Remove abilities from old weapon
		if(ActorInfo.IsNetAuthority())
			ActorInfo.Inventory.Get()->RemoveAbilities();
	}
	if(NewWeapon)
	{
		NewWeapon->GetFP_Mesh()->SetVisibility(true);

		// Add abilities from new weapon
		if(ActorInfo.IsNetAuthority())
			ActorInfo.Inventory.Get()->GiveAbilities(NewWeapon);
	}*/
}

void UEquipAbility::Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	Super::Client_PredictionFailed_Implementation(ActorInfo);

	PRINTLINE;
	// Sets current equipped weapon to last equipped index and stops all anim montages
	//ActorInfo.Inventory.Get()->SetCurrentIndex(ActorInfo.Inventory.Get()->GetLastIndex());
	Equip(ActorInfo.Inventory.Get()->GetLastIndex(), ActorInfo);
	if(UAnimInstance* Anim = ActorInfo.Character.Get()->GetFP_Mesh()->GetAnimInstance())
		Anim->Montage_Stop(0.f);
}


int32 UEquipAbility::EquipWeapon(UAbilitySystemComponent* ASC, int32 Index)
{
	if(!ASC) return 0;
	FGameplayEventData Data;
	Data.Target = ASC->GetOwner();
	Data.EventMagnitude = Index;
	return ASC->HandleGameplayEvent(TAG("Event.Equip"), &Data);
}


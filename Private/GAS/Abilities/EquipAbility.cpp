// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/EquipAbility.h"

#include "Character/ShooterCharacter.h"
#include "GAS/Abilities/AbilityTasks/AbilityTask_WaitMontageCompleted.h"

UEquipAbility::UEquipAbility()
{
	OwnedAbilities.Add(UEquipAbility_Server::StaticClass());
	
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

	if(!IsCorrectNetActivation(ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}
	
	if(TriggerEventData && ActorInfo)
	{
		const int32 Index = TriggerEventData->EventMagnitude;
		if(INVENTORY->GetWeapons().IsValidIndex(Index))
		{// If valid index, equip
			INVENTORY->SetCurrentIndex(Index);
			AWeapon* NewWeapon = INVENTORY->GetWeapons()[Index];
			if(NewWeapon) // Add equipping loose gameplay tags
			{
				if(NewWeapon == CHARACTER->GetCurrentWeapon())
				{// If attempting to equip the current weapon, return
					EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
					return;
				}
				GET_ASC->AddLooseGameplayTagForDurationSingle(TAG("Status.State.Equipping"), NewWeapon->GetWeaponSwapDuration());
			}
			
			Equip(NewWeapon, INVENTORY->GetCurrent(), *(FGameplayAbilityActorInfoExtended*)ActorInfo);
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
		else
		{// If invalid index, equip nothing
			Equip(nullptr, INVENTORY->GetCurrent(), *(FGameplayAbilityActorInfoExtended*)ActorInfo);
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
	}
	else EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UEquipAbility::Equip_Implementation(AWeapon* NewWeapon, AWeapon* OldWeapon, const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	// Remove any tags related to weapon state
	ActorInfo.ASC.Get()->RemoveLooseGameplayTagChildren(TAG("WeaponState"));

	// Set current weapon
	ActorInfo.Character.Get()->SetCurrentWeapon(NewWeapon);
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
	}
}

void UEquipAbility::Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	Super::Client_PredictionFailed_Implementation(ActorInfo);
	/*
	UCharacterInventoryComponent* Inventory = ActorInfo.Inventory.Get();
	if(Inventory->GetWeapons().IsValidIndex(Inventory->GetLastIndex()))
	{
		Inventory->SetCurrentIndex(Inventory->GetLastIndex());
		Equip(Inventory->GetWeapons()[Inventory->GetLastIndex()], Inventory->GetCurrent(), ActorInfo);
		
		if(UAnimInstance* AnimInstance = ActorInfo.Character.Get()->GetFP_Mesh()->GetAnimInstance())
			AnimInstance->Montage_Stop(0.f);
	}
	else PRINT(TEXT("Failed"));*/

	ActorInfo.Inventory.Get()->SetCurrentIndex(ActorInfo.Inventory.Get()->GetLastIndex());
	Equip(ActorInfo.Inventory.Get()->GetCurrent(), ActorInfo.Inventory.Get()->GetWeapons()[ActorInfo.Inventory.Get()->GetLastIndex()], ActorInfo);
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


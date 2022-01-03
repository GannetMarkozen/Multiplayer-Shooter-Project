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

	ActivationOwnedTags.AddTag(TAG("Status.State.Equipping"));

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
		if(INVENTORY->GetWeapons().IsValidIndex(Index))
		{
			INVENTORY->SetCurrentIndex(Index);
			Equip(INVENTORY->GetWeapons()[Index], INVENTORY->GetCurrent(), (FGameplayAbilityActorInfoExtended&)*ActorInfo);
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UEquipAbility::Equip(AWeapon* NewWeapon, AWeapon* OldWeapon, const FGameplayAbilityActorInfoExtended& ActorInfo)
{
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
		ActorInfo.Character.Get()->SetCurrentWeapon(NewWeapon);

		// Add abilities from new weapon
		if(ActorInfo.IsNetAuthority())
			ActorInfo.Inventory.Get()->GiveAbilities(NewWeapon);
	}
}

void UEquipAbility::Client_PredictionSucceeded_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	Super::Client_PredictionSucceeded_Implementation(ActorInfo);
}


void UEquipAbility::Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo)
{
	if(UAnimInstance* AnimInstance = ActorInfo.Character.Get()->GetFP_Mesh()->GetAnimInstance())
		AnimInstance->Montage_Stop(0.f);
	
	const UCharacterInventoryComponent* Inventory = ActorInfo.Inventory.Get();
	if(Inventory->GetWeapons().IsValidIndex(Inventory->GetLastIndex()))
		Equip(Inventory->GetWeapons()[Inventory->GetLastIndex()], Inventory->GetCurrent(), ActorInfo);
}


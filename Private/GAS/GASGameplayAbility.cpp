// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GASGameplayAbility.h"
#include "Character/ShooterCharacter.h"
#include "GameplayAbilities/Public/GameplayCueManager.h"

UGASGameplayAbility::UGASGameplayAbility()
{
	CooldownGameplayEffectClass = UCooldownEffect::StaticClass();
}


void UGASGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// Give character owned abilities
	if(ActorInfo->IsNetAuthority())
	for(const TSubclassOf<UGASGameplayAbility>& Class : OwnedAbilities)
	{
		const FGameplayAbilitySpec AbilitySpec(Class, 1, static_cast<int32>(Class.GetDefaultObject()->Input), this);
		GET_ASC->GiveAbility(AbilitySpec);
	}
	
	// BP implementation of OnGiveAbility
	K2_OnGiveAbility(*(FGameplayAbilityActorInfoExtended*)ActorInfo, Spec);
}

void UGASGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	// Remove owned abilities from character
	if(ActorInfo->IsNetAuthority())
	for(const TSubclassOf<UGASGameplayAbility>& Class : OwnedAbilities)
	{
		if(const FGameplayAbilitySpec* OwnedSpec = GET_ASC->FindAbilitySpecFromClass(Class))
		{
			GET_ASC->CancelAbilityHandle(OwnedSpec->Handle);
			GET_ASC->SetRemoveAbilityOnEnd(OwnedSpec->Handle);
		}
	}
	
	// BP implementation of OnRemoveAbility
	K2_OnRemoveAbility(*(FGameplayAbilityActorInfoExtended*)ActorInfo, Spec);
}


UCharacterMovementComponent* UGASGameplayAbility::GetCharacterMovement() const
{
	return GetActorInfoExtended()->Character.Get()->GetCharacterMovement();
}
/*
const FGameplayTagContainer* UGASGameplayAbility::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset(); // MutableTags writes to the TempCooldownTags on the CDO so clear it in case the ability cooldown tags change (moved to a different slot)
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if(ParentTags)
	{
		MutableTags->AppendTags(*ParentTags);
	}
	MutableTags->AppendTags(CooldownTags);
	return MutableTags;
}

void UGASGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTags);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Cooldown")), CooldownDuration.GetValueAtLevel(GetAbilityLevel()));
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}
*/


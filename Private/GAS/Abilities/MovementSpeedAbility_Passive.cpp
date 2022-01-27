// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementSpeedAbility_Passive.h"

#include "Character/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"


UMovementSpeedAbility_Passive::UMovementSpeedAbility_Passive()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
}

void UMovementSpeedAbility_Passive::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// Bind on movement speed changed to MovementSpeedChanged and init max walk speed to current movement speed attribute value
	GET_ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetMovementSpeedAttribute()).AddUObject(this, &UMovementSpeedAbility_Passive::MovementSpeedChanged);
	CHARACTER->GetCharacterMovement()->MaxWalkSpeed = GET_ASC->GetSet<UCharacterAttributeSet>()->MovementSpeed.GetCurrentValue();
}

void UMovementSpeedAbility_Passive::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool UMovementSpeedAbility_Passive::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* GameplayTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, GameplayTags, TargetTags, OptionalRelevantTags);
}

void UMovementSpeedAbility_Passive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMovementSpeedAbility_Passive::MovementSpeedChanged_Implementation(const FOnAttributeChangeData& Data)
{
	GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

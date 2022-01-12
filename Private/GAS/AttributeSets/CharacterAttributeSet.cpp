// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSets/CharacterAttributeSet.h"

#include "Character/ShooterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UCharacterAttributeSet::UCharacterAttributeSet()
{
	
}

void UCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCharacterAttributeSet, BulletResistance, COND_None, REPNOTIFY_Always);
}

void UCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}


void UCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	const FGameplayAttributeData* Data = Attribute.GetGameplayAttributeDataChecked(this);
	if(Data == &Health)
	{// Clamped max attributes
		if(MaxHealth.GetCurrentValue() > 0.f) NewValue = FMath::Clamp<float>(NewValue, 0.f, MaxHealth.GetCurrentValue());
	}
	if(Data == &MovementSpeed || Data == &MaxHealth)
	{// Non-negative number attributes
		NewValue = FMath::Max<float>(0.f, NewValue);
	}
	else if(Data == &BulletResistance || Data == &ExplosionResistance)
	{// Percentage attributes
		NewValue = FMath::Clamp<float>(NewValue, 0.f, 100.f);
	}
}


void UCharacterAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, Health, OldHealth);
}

void UCharacterAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, MaxHealth, OldMaxHealth);
}

void UCharacterAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, MovementSpeed, OldMovementSpeed);
}

void UCharacterAttributeSet::OnRep_BulletResistance(const FGameplayAttributeData& OldBulletResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, BulletResistance, OldBulletResistance);
}

void UCharacterAttributeSet::OnRep_ExplosionResistance(const FGameplayAttributeData& OldExplosionResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCharacterAttributeSet, ExplosionResistance, OldExplosionResistance);
}



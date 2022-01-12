// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DeathEffect.h"

#include "Character/ShooterCharacter.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"

UDeathEffect::UDeathEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UCharacterAttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::Override;
	Modifier.Magnitude = 0.f;
	Modifiers.Add(Modifier);

	InheritableOwnedTagsContainer.Added.AddTag(TAG("Status.State.Dead"));
	GrantedApplicationImmunityTags.IgnoreTags.AddTag(TAG("Status.State.Dead"));
}



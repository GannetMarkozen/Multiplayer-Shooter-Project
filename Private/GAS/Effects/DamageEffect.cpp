// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DamageEffect.h"

#include "GAS/GASAttributeSet.h"
#include "GAS/Effects/DamageExecutionCalculation.h"

UDamageEffect::UDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayEffectExecutionDefinition Definition;
	Definition.CalculationClass = UDamageExecutionCalculation::StaticClass();
	Executions.Add(Definition);

	ApplicationTagRequirements.IgnoreTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.State.Dead")));
	ApplicationTagRequirements.IgnoreTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.State.Invincible")));
}

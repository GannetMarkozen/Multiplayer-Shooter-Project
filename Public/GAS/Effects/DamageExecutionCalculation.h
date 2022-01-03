// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilities/Public/GameplayEffectExecutionCalculation.h"
#include "GAS/ExtendedTypes.h"
#include "DamageExecutionCalculation.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UDamageExecutionCalculation();
	
protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	virtual float CalculateBaseDamage(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FAggregatorEvaluateParameters& EvaluationParams) const;

	// Returns all hit results on the target this effect is being applied onto
	virtual void GetHits(TArray<const FHitResult*>& OutHits, const FGameplayEffectCustomExecutionParameters& ExecutionParams) const;

	float DefaultHeadshotMultiplier;
};

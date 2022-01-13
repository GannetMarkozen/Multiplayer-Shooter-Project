// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitscanWeapon.generated.h"

/** Contains the parameters and calculation requirements for a hitscan weapon
 * including damage falloff and line trace ability
 */
UCLASS(Abstract)
class MULTIPLAYERSHOOTER_API AHitscanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	AHitscanWeapon();
	
protected:
	// Curve float determining the damage at a given range in terms of a multiplier to the BaseDamage, 1 being full BaseDamage. Range of 1 == max range on the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|DamageCalculation")
	TObjectPtr<class UCurveFloat> DamageFalloffCurve;

	// Calculate damage from weapon. Does not take into account headshots and other properties that would be further calculated inside of a
	// Gameplay Effect Execution Calculation class. This implementation is primarily used to allow damaging other objects like bottles or
	// crates without using the effect execution with advanced calculations like headshots
	virtual int32 CalculateDamage_Implementation(const class AActor* Target, const FGameplayEffectSpecHandle& Spec) const override;
};

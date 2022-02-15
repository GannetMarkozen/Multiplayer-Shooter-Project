// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileWeapon.h"
#include "GAS/GASGameplayAbility.h"
#include "HitscanWeapon.generated.h"

/** Contains the parameters and calculation requirements for a hitscan weapon
 * including damage falloff and line trace ability
 */ 
UCLASS(Abstract)
class MULTIPLAYERSHOOTER_API AHitscanWeapon : public AProjectileWeapon
{
	GENERATED_BODY()
public:
	AHitscanWeapon();
	
protected:
	virtual void SetupInputBindings() override;
	virtual int32 CalculateDamage_Implementation(const class AActor* Target, const FGameplayEffectSpecHandle& Spec) const override;
	virtual void OnFireWeapon_Implementation(const FGameplayAbilityTargetDataHandle& TargetData) override;
	
	// Curve float determining the damage at a given range in terms of a multiplier to the BaseDamage, 1 being full BaseDamage. Range of 1 == max range on the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations|Damage Calculation")
	class UCurveFloat* DamageFalloffCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations|Cosmetic")
	class UParticleSystem* BulletTracer;

	FTimerHandle BulletTracerTimerHandle;

	virtual void FireWeapon() { PRINT(TEXT("Fired")); }
};

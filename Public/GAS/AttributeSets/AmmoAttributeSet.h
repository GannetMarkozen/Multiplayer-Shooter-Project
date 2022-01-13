// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASAttributeSet.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "AmmoAttributeSet.generated.h"


// Helper macros from Attribute.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** Reserve ammo for all the different
 *  ammo types
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UAmmoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UAmmoAttributeSet();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	//virtual void Attribute
	
public:	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RifleAmmo, Category = "Attributes")
	FGameplayAttributeData RifleAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, RifleAmmo);
	UFUNCTION()
	virtual void OnRep_RifleAmmo(const FGameplayAttributeData& OldRifleAmmo);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ShotgunAmmo, Category = "Attributes")
	FGameplayAttributeData ShotgunAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, ShotgunAmmo);
	UFUNCTION()
	virtual void OnRep_ShotgunAmmo(const FGameplayAttributeData& OldShotgunAmmo);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PistolAmmo, Category = "Attributes")
	FGameplayAttributeData PistolAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, PistolAmmo);
	UFUNCTION()
	virtual void OnRep_PistolAmmo(const FGameplayAttributeData& OldPistolAmmo);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ExplosiveAmmo, Category = "Attributes")
	FGameplayAttributeData ExplosiveAmmo;
	ATTRIBUTE_ACCESSORS(UAmmoAttributeSet, ExplosiveAmmo);
	UFUNCTION()
	virtual void OnRep_ExplosiveAmmo(const FGameplayAttributeData& OldExplosiveAmmo);
};

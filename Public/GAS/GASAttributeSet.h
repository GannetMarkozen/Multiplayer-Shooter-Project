// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilities/Public/AttributeSet.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "MultiplayerShooter/MultiplayerShooter.h"

#include "GASAttributeSet.generated.h"

// Helper macros from Attribute.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UGASAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UGASAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGASAttributeSet, Health)
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "Attributes")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UGASAttributeSet, MovementSpeed);
	UFUNCTION()
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BulletResistance, Category = "Attributes")
	FGameplayAttributeData BulletResistance;
	ATTRIBUTE_ACCESSORS(UGASAttributeSet, BulletResistance);
	UFUNCTION()
	virtual void OnRep_BulletResistance(const FGameplayAttributeData& OldBulletResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ExplosionResistance, Category = "Attributes")
	FGameplayAttributeData ExplosionResistance;
	ATTRIBUTE_ACCESSORS(UGASAttributeSet, ExplosionResistance);
	UFUNCTION()
	virtual void OnRep_ExplosionResistance(const FGameplayAttributeData& OldExplosionResistance);
};

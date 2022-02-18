// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "Objects/ReplicatedObject.h"
#include "FiringObject.generated.h"

/**
 * 
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class MULTIPLAYERSHOOTER_API UFiringObject : public UReplicatedObject
{
	GENERATED_BODY()
public:
	UFiringObject(){}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon Firing")
	void StartFiring();
	virtual FORCEINLINE void StartFiring_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon Firing")
	void StopFiring();
	virtual FORCEINLINE void StopFiring_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Weapon Firing")
	bool CanFire() const;
	virtual FORCEINLINE bool CanFire_Implementation() const { return true; }

	static const FGameplayTag FiringStateTag;
};

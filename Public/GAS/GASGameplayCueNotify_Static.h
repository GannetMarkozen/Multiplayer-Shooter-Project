// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilities/Public/GameplayCueNotify_Static.h"
#include "GASGameplayCueNotify_Static.generated.h"

UENUM(BlueprintType)
enum class EGameplayCueNetExecutionPolicy : uint8
{
	// Regular multicast
	NetMulticast,
	// Local only, will RPC down to instigator's client if activated non-locally server-side
	LocalInstigator,
	// Local only, will RPC down to target's client if activated non-locally server-side
	LocalTarget
};
/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UGASGameplayCueNotify_Static : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
public:
	UGASGameplayCueNotify_Static(){}

	FORCEINLINE EGameplayCueNetExecutionPolicy GetNetExecutionPolicy() const { return NetExecutionPolicy; }

protected:
	// Determines how this gameplay cue will be multicasted. Only applies to effects at the moment
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Cue")
	EGameplayCueNetExecutionPolicy NetExecutionPolicy = EGameplayCueNetExecutionPolicy::NetMulticast;
};

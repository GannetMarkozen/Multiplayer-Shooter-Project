// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameFramework/Actor.h"
#include "LineTraceObject.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class MULTIPLAYERSHOOTER_API ULineTraceObject : public UObject
{
	GENERATED_BODY()
public:
	// Does a line trace with the pre-configured line trace settings
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "OwningAbility"))
	FHitResult DoLineTrace(const FVector& Location, const FRotator& Rotation, const TArray<class AActor*>& IgnoreActors, float SpreadMagnitude = 1.f, bool bDrawDebug = false);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Configurations")
	float Range = 50000.f;
	
protected:
	// In degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	FVector2D Spread = {0.f, 0.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};

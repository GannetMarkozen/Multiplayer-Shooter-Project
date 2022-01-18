// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FPAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UFPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFPAnimInstance();

protected:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class AShooterCharacter* Character;

	// The transform offset of the IK arms mesh
	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	FTransform OffsetTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	float ADSMagnitude = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	FRotator LastAimRotation;
};

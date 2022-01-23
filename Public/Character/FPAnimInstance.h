﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	virtual void Init();

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "Init"), Category = "Anim")
	void BP_Init();

	virtual void SetIKTransforms();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim")
	void WeaponChanged(class AWeapon* NewWeapon, const class AWeapon* OldWeapon);
	virtual void WeaponChanged_Implementation(class AWeapon* NewWeapon, const class AWeapon* OldWeapon);

	UFUNCTION(BlueprintNativeEvent, Meta = (DisplayName = "Set Variables"), Category = "Anim")
	void SetVars(const float DeltaTime);
	virtual void SetVars_Implementation(const float DeltaTime);

	virtual void CalculateWeaponSway(const float DeltaTime);
	
public:	
	/*
	 *	References
	 */
	
	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class AShooterCharacter* Character;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class AWeapon* CurrentWeapon;

	/*
	 *	Params
	 */
	UPROPERTY(EditAnywhere, Category = "Configurations")
	class UCurveVector* MovementWeaponSwayCurve;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	class UCurveVector* IdleWeaponSwayCurve;

	/*
	 *	IK / ADS metrics
	 */

	// The current pose applied to the IK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|IK")
	class UAnimSequence* AnimPose;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FTransform RHandToSightsTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FTransform RelativeAimPointTransform;
	
	// The transform offset of the IK arms mesh
	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FTransform OffsetTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	float ADSMagnitude = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FRotator LastAimRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FVector LastVelocity;

	/*
	 *	Accumulative values
	 */
	
	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FRotator AccumulativeRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FRotator AccumulativeRotationInterp;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FVector VelocityTarget;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FVector VelocityInterp;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	float MovementSpeedInterp = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	float MovementWeaponSwayProgressTime = 0.f;

	/*
	 *	Basic locomotion metrics
	 */
	
	UPROPERTY(BlueprintReadWrite, Category = "Anim|Locomotion")
	float MovementDirection = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|Locomotion")
	bool bIsFalling = false;

	/*
	 * Configurations
	 */

	UPROPERTY(EditAnywhere, Category = "Configurations")
	float MaxMoveSpeed = 600.f;

	// Determines the speed the rotation transform target is reset
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float AccumulativeRotationReturnInterpSpeed = 30.f;

	// Determines the speed the rotation transform reaches it's target
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float AccumulativeRotationInterpSpeed = 5.f;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	float VelocityInterpSpeed = 10.f;
};

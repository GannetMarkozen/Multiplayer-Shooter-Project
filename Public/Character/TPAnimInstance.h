// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UTPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UTPAnimInstance();

protected:
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
	 *	REFERENCES
	 */
	
	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class AShooterCharacter* Character;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class AWeapon* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|IK")
	class UAnimSequence* AnimPose;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	float CurrentAimPointOffset = 0.f;

	/*
	 *	IK
	 */

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FTransform RHandToSightsTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FTransform RelativeCameraTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	FTransform OffsetTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	float ADSMagnitude = 0.f;

	/*
	 *	BASIC LOCOMOTION
	 */
	
	UPROPERTY(BlueprintReadWrite, Category = "Anim|Locomotion")
	float MovementDirection = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|Locomotion")
	bool bIsFalling = false;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|Locomotion")
	float MovementWeaponSwayProgress = 0.f;

	/*
	 *	ACCUMULATIVE OFFSETS
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
	 *	OTHER VARS
	 */

	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FTransform CameraTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FVector LastVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
	FRotator LastAimRotation;
	
	/*
	*	CONFIGURATIONS
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	float MaxMoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	class UCurveVector* IdleWeaponSwayCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	class UCurveVector* MovementWeaponSwayCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	float AccumulativeRotationReturnInterpSpeed = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	float AccumulativeRotationInterpSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	float VelocityInterpSpeed = 10.f;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TrueFPSAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UTrueFPSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UTrueFPSAnimInstance();

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

	virtual void CalculateMeshOffset(const float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Anim")
	virtual void OnCharacterLanded(class AShooterCharacter* InCharacter, const FHitResult& Hit);

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "On Character Landed"), Category = "Anim")
	void BP_OnCharacterLanded(class AShooterCharacter* InCharacter, const FHitResult& Hit);
	
	

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
	 *	STATIONARY ANIM 
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Stationary")
	float RootYawOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Stationary")
	bool bIsTurningInPlace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Stationary")
	float StationaryYawThreshold = 89.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Stationary")
	float StationaryVelocityThreshold = 10.f;

	// The stationary rotation speed and direction (negative == left positive == right) ranging from -8 to 8
	float StationaryYawInterpSpeed = 0.f;

	// The current yaw rotation speed of this frame. Multiply play rate based on this value
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Stationary")
	float StationaryYawSpeedNormal = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Stationary")
	float StationaryYawAmount = 0.f;

	/*
	 *	IK
	 */

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|IK")
	FTransform RHandToSightsTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|IK")
	FTransform RelativeCameraTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|IK")
	FTransform OffsetTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|IK")
	float ADSMagnitude = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Anim|IK")
	float CurrentWeightScale = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|IK")
	FTransform CurrentCustomWeaponOffsetTransform;

	/*
	 *	BASIC LOCOMOTION
	 */
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Locomotion")
	float MovementDirection = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Locomotion")
	float MovementVelocity = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Locomotion")
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

	// The rotation interp speed on non-local players' viewports
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
	float NonLocalCameraRotationInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	float VelocityInterpSpeed = 10.f;
};

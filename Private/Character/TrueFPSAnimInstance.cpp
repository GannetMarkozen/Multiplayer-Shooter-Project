// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TrueFPSAnimInstance.h"

#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Widgets/Text/ISlateEditableTextWidget.h"


UTrueFPSAnimInstance::UTrueFPSAnimInstance()
{
	
}

void UTrueFPSAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UTrueFPSAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if(!Character)
	{
		Character = Cast<AShooterCharacter>(TryGetPawnOwner());
		if(Character)
		{
			Init();
			BP_Init();
		}
		else
		{
			return;
		}
	}
	
	SetVars(DeltaTime);
	CalculateMeshOffset(DeltaTime);
	CalculateWeaponSway(DeltaTime);

	// Set old vars for next anim update
	LastAimRotation = CameraTransform.Rotator();
	LastVelocity = Character->GetCharacterMovement()->Velocity;
}

void UTrueFPSAnimInstance::SetVars_Implementation(const float DeltaTime)
{
	if(Character->IsLocallyControlled())
	{
		CameraTransform = Character->GetCamera()->GetComponentTransform();
	}
	else
	{
		// Clamp pitch to prevent weird yaw rotation glitch on simulated proxies
		constexpr float ClampAngle = 89.9f;
		FRotator ClampedBaseAimRotation = Character->GetBaseAimRotation();
		ClampedBaseAimRotation.Pitch = FMath::ClampAngle(ClampedBaseAimRotation.Pitch, -ClampAngle, ClampAngle);

		// Interp camera rotation to prevent a hitchy look on non-local viewports
		const FRotator& RotationInterp = UKismetMathLibrary::RInterpTo(CameraTransform.Rotator(), ClampedBaseAimRotation, DeltaTime, NonLocalCameraRotationInterpSpeed);
		CameraTransform = FTransform(RotationInterp, Character->GetCamera()->GetComponentLocation());
	}

	// Used for mesh calculations
	const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("root"));
	RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);
	
	/*
	 *	LOCOMOTION VARS
	 */

	MovementDirection = CalculateDirection(Character->GetCharacterMovement()->Velocity, FRotator(0.f, Character->GetBaseAimRotation().Yaw, 0.f));

	MovementVelocity = Character->GetCharacterMovement()->Velocity.Size();

	bIsFalling = Character->GetCharacterMovement()->IsFalling();

	MovementWeaponSwayProgressTime += DeltaTime * (MovementSpeedInterp / MaxMoveSpeed);

	/*
	 *	IK VARS
	 */
	
	ADSMagnitude = Character->ADSValue;
	
	//const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	//RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);
	

	/*
	 *	ACCUMULATIVE OFFSET VARS
	 */

	constexpr float AngleClamp = 6.f;
	const FRotator& AddRotation = Character->GetBaseAimRotation() - LastAimRotation;
	FRotator AddRotationClamped = FRotator(FMath::ClampAngle(AddRotation.Pitch, -AngleClamp, AngleClamp) * 1.5f,
		FMath::ClampAngle(AddRotation.Yaw, -AngleClamp, AngleClamp), 0.f);
	AddRotationClamped.Roll = AddRotationClamped.Yaw * 0.7f;

	AccumulativeRotation += AddRotationClamped;
	AccumulativeRotation = UKismetMathLibrary::RInterpTo(AccumulativeRotation, FRotator::ZeroRotator, DeltaTime, AccumulativeRotationReturnInterpSpeed);
	AccumulativeRotationInterp = UKismetMathLibrary::RInterpTo(AccumulativeRotationInterp, AccumulativeRotation, DeltaTime, AccumulativeRotationInterpSpeed);

	MovementSpeedInterp = UKismetMathLibrary::FInterpTo(MovementSpeedInterp, bIsFalling ? 0.f : Character->GetCharacterMovement()->Velocity.Size(), DeltaTime, 3.f);

	const FVector& Velocity = Character->GetCharacterMovement()->Velocity;
	const FVector& Difference = Velocity - LastVelocity;
	
	VelocityTarget = UKismetMathLibrary::VInterpTo(VelocityTarget, Character->GetCharacterMovement()->Velocity, DeltaTime, VelocityInterpSpeed);
	if(Difference.Size() > 400.f)
	{
		const FVector& OrientationDifference = UKismetMathLibrary::RotateAngleAxis(Velocity - LastVelocity * 10.f, Character->GetControlRotation().Yaw, FVector(0.f, 0.f, 1.f));//UKismetMathLibrary::Quat_RotateVector(FRotator(0.f, Character->GetControlRotation().Yaw, 0.f).Quaternion(), Velocity - LastVelocity * 10.f);
		const FVector& ClampedDifference =  UKismetMathLibrary::ClampVectorSize(OrientationDifference, 0.f, 30000.f);
		VelocityTarget += FVector(ClampedDifference.X / 3.f, ClampedDifference.Y / 3.f, ClampedDifference.Z);
	}
	
	VelocityInterp = UKismetMathLibrary::VInterpTo(VelocityInterp, VelocityTarget, DeltaTime, 3.f);
}

void UTrueFPSAnimInstance::CalculateWeaponSway(const float DeltaTime)
{
	// Add onto offsets and then apply onto offset transform for IK
	FVector OffsetLocation = FVector::ZeroVector;
	FRotator OffsetRotation = FRotator::ZeroRotator;

	// Get inverse to apply the opposite of the rotational influence to the weapon sway
	const FRotator& AccumulativeRotationInterpInverse = AccumulativeRotationInterp.GetInverse();

	// Apply location offset from accumulative rotation inverse
	OffsetLocation += FVector(0.f, AccumulativeRotationInterpInverse.Yaw, AccumulativeRotationInterpInverse.Pitch) / 6.f;

	// Apply location offset from interp orientation velocity
	const FVector& OrientationVelocityInterp = (FRotator(0.f, CameraTransform.Rotator().Yaw, 0.f) - VelocityInterp.Rotation()).Vector() * VelocityInterp.Size() * FVector(1.f, -1.f, 1.f);
	const FVector& MovementOffset = (OrientationVelocityInterp / MaxMoveSpeed) * FMath::Max<float>(1.f - ADSMagnitude, 0.6f);
	OffsetLocation += MovementOffset;

	// Add accumulative rotation
	OffsetRotation += AccumulativeRotationInterpInverse;

	// Add movement offset to rotation
	OffsetRotation += FRotator(MovementOffset.Z * 5.f, MovementOffset.Y, MovementOffset.Y * 2.f);

	// Apply weight scale of weapon to offsets before weapon sway curves
	OffsetLocation *= CurrentWeightScale;
	OffsetRotation.Pitch *= CurrentWeightScale;
	OffsetRotation.Yaw *= CurrentWeightScale;
	OffsetRotation.Roll *= CurrentWeightScale;

	// Apply idle vector curve anim to offset location
	if(IdleWeaponSwayCurve)
		OffsetLocation += IdleWeaponSwayCurve->GetVectorValue(GetWorld()->GetTimeSeconds()) * 8.f * FMath::Max<float>(1.f - ADSMagnitude, 0.1f);

	// Apply movement offset to offset location
	if(MovementWeaponSwayCurve)
		OffsetLocation += MovementWeaponSwayCurve->GetVectorValue(MovementWeaponSwayProgressTime) * (MovementSpeedInterp / MaxMoveSpeed) * 5.f * FMath::Max<float>(1.f - ADSMagnitude, 0.1f);
	
	OffsetTransform = Character->WeaponOffsetTransform * FTransform(OffsetRotation, OffsetLocation, FVector(1.f));
	//UE_LOG(LogTemp, Warning, TEXT("OffsetTransform == %s"), *FTransform(OffsetRotation, OffsetLocation, FVector(1.f)).ToString());
}


void UTrueFPSAnimInstance::CalculateMeshOffset(const float DeltaTime)
{
	if(!bIsTurningInPlace && abs(RootYawOffset) < 2.f && Character->GetCharacterMovement()->Velocity.Size() < StationaryVelocityThreshold)
	{
		bIsTurningInPlace = true;
	}

	if(bIsTurningInPlace)
	{
		RootYawOffset += FRotator::NormalizeAxis(LastAimRotation.Yaw - CameraTransform.Rotator().Yaw);

		// If exceeded rotation or velocity thresholds, set turn in place to false and set rot speed to desired speed
		if(Character->GetCharacterMovement()->Velocity.Size() >= StationaryVelocityThreshold)
		{
			bIsTurningInPlace = false;
			StationaryYawInterpSpeed = 8.f;
		}
		else if(abs(RootYawOffset) >= StationaryYawThreshold)
		{
			bIsTurningInPlace = false;
			StationaryYawInterpSpeed = 5.f;
		}

		// If no longer turning in place, set the rotation amount for turning animation usage
		if(!bIsTurningInPlace)
		{
			StationaryYawAmount = -RootYawOffset;
		}
	}

	if(!bIsTurningInPlace && RootYawOffset)
	{
		const float YawDifference = FRotator::NormalizeAxis(LastAimRotation.Yaw - CameraTransform.Rotator().Yaw);
		RootYawOffset += YawDifference;
		
		if(-YawDifference > 0.f == StationaryYawAmount > 0.f)
			StationaryYawAmount += -YawDifference;

		StationaryYawSpeedNormal = FMath::Clamp<float>(StationaryYawAmount / 180.f, 1.5f, 3.f);
            
		// Never allow the yaw offset to exceed the yaw threshold
		RootYawOffset = FMath::ClampAngle(RootYawOffset, -StationaryYawThreshold, StationaryYawThreshold);
            
		// Never allow yaw offset to exceed yaw threshold
		RootYawOffset = UKismetMathLibrary::FInterpTo(RootYawOffset, 0.f, DeltaTime, StationaryYawInterpSpeed);

		// Once matched rotation, clear vars
		if(abs(RootYawOffset) < 2.f)
		{
			StationaryYawInterpSpeed = 0.f;
			StationaryYawAmount = 0.f;
			RootYawOffset = 0.f;
		}
	}
}



void UTrueFPSAnimInstance::Init()
{
	Mesh = Character->GetMesh();

	Character->GetCharacterInventory()->CurrentWeaponChangeDelegate.AddDynamic(this, &UTrueFPSAnimInstance::WeaponChanged);
	Character->LandedMultiDelegate.AddDynamic(this, &UTrueFPSAnimInstance::OnCharacterLanded);

	if(AWeapon* NewWeapon = Character->GetCurrentWeapon())
		WeaponChanged(NewWeapon, nullptr);
}

void UTrueFPSAnimInstance::WeaponChanged_Implementation(AWeapon* NewWeapon, const AWeapon* OldWeapon)
{
	CurrentWeapon = NewWeapon;
	if(CurrentWeapon)
	{
		// Init vars from current weapon
		AnimPose = CurrentWeapon->AnimPose;
		CurrentWeightScale = CurrentWeapon->WeightScale;
		CurrentAimPointOffset = CurrentWeapon->AimOffset;
		CurrentCustomWeaponOffsetTransform = CurrentWeapon->CustomWeaponOffsetTransform;

		// Get the sight transform relative to right hand for control rig calculations
		RHandToSightsTransform = CurrentWeapon->GetSightsWorldTransform().GetRelativeTransform(Mesh->GetSocketTransform(FName("hand_r")));
	}
	else
	{
		// Clear current weapon vars
		AnimPose = nullptr;
		CurrentWeightScale = 0.f;
		CurrentAimPointOffset = 0.f;
		CurrentCustomWeaponOffsetTransform = FTransform::Identity;
		RHandToSightsTransform = FTransform::Identity;
	} 
}

void UTrueFPSAnimInstance::OnCharacterLanded(AShooterCharacter* InCharacter, const FHitResult& Hit)
{
	
	BP_OnCharacterLanded(InCharacter, Hit);
}


void UTrueFPSAnimInstance::SetIKTransforms()
{
	RHandToSightsTransform = CurrentWeapon->GetSightsWorldTransform().GetRelativeTransform(Mesh->GetSocketTransform(FName("hand_r")));

	// Init vars
	CurrentAimPointOffset = CurrentWeapon->AimOffset;
	CurrentCustomWeaponOffsetTransform = CurrentWeapon->CustomWeaponOffsetTransform;
	
	//RHandToSightsTransform = CurrentWeapon->GetTP_Mesh()->GetSocketTransform(SightsSocketName).GetRelativeTransform(Mesh->GetSocketTransform("hand_r"));

	/*const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	FTransform CameraOffset = Character->GetCamera()->GetComponentTransform().GetRelativeTransform(RootOffset);
	CameraOffset.AddToTranslation(CameraOffset.GetRotation().GetForwardVector() * AimPointOffset);
	RelativeAimPointTransform = CameraOffset;*/

	//const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	//RelativeCameraTransform = Character->GetCamera()->GetComponentTransform().GetRelativeTransform(RootOffset);
}





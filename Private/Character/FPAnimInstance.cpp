﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/FPAnimInstance.h"

#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


UFPAnimInstance::UFPAnimInstance()
{
	
}

void UFPAnimInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}


void UFPAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

}

void UFPAnimInstance::NativeUpdateAnimation(float DeltaTime)
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
	CalculateWeaponSway(DeltaTime);

	// Set last aim rotation for next update
	LastAimRotation = Character->GetBaseAimRotation();
	LastVelocity = Character->GetCharacterMovement()->Velocity;
}

void UFPAnimInstance::Init()
{
	Mesh = Character->GetFP_Mesh();
	
	// Bind weapon changed delegate and call if immediately if current weapon is valid
	Character->GetCharacterInventory()->CurrentWeaponChangeDelegate.AddDynamic(this, &UFPAnimInstance::WeaponChanged);
	
	if(AWeapon* NewWeapon = Character->GetCurrentWeapon())
		WeaponChanged(NewWeapon, nullptr);
}

void UFPAnimInstance::SetVars_Implementation(const float DeltaTime)
{
	// Basic vars
	MovementDirection = CalculateDirection(Character->GetCharacterMovement()->Velocity, Character->GetBaseAimRotation());
	
	bIsFalling = Character->GetCharacterMovement()->IsFalling();
	
	MovementWeaponSwayProgressTime += DeltaTime * (MovementSpeedInterp / MaxMoveSpeed);
	
	ADSMagnitude = Character->ADSValue;

	/*
	 *	WeaponSway / IK stuff
	 */
	
	const FRotator& AddRotation = Character->GetBaseAimRotation() - LastAimRotation;
	FRotator AddRotationClamped = FRotator(FMath::ClampAngle(AddRotation.Pitch, -25.f, 25.f) * 1.5f,
		FMath::ClampAngle(AddRotation.Yaw, -25.f, 25.f), 0.f);
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


void UFPAnimInstance::CalculateWeaponSway(const float DeltaTime)
{
	// Add onto offsets and then apply onto offset transform for IK
	FVector OffsetLocation = FVector::ZeroVector;
	FRotator OffsetRotation = FRotator::ZeroRotator;

	// Apply idle vector curve anim to offset location
	if(IdleWeaponSwayCurve)
		OffsetLocation += IdleWeaponSwayCurve->GetVectorValue(GetWorld()->GetTimeSeconds()) * 8.f * FMath::Max<float>(1.f - ADSMagnitude, 0.1f);

	// Apply movement offset to offset location
	if(MovementWeaponSwayCurve)
		OffsetLocation += MovementWeaponSwayCurve->GetVectorValue(MovementWeaponSwayProgressTime) * (MovementSpeedInterp / MaxMoveSpeed) * 5.f * FMath::Max<float>(1.f - ADSMagnitude, 0.1f);

	// Get inverse to apply the opposite of the rotational influence to the weapon sway
	const FRotator& AccumulativeRotationInterpInverse = AccumulativeRotationInterp.GetInverse();

	// Apply location offset from accumulative rotation inverse
	OffsetLocation += FVector(0.f, AccumulativeRotationInterpInverse.Yaw, AccumulativeRotationInterpInverse.Pitch) / 6.f;

	// Apply location offset from interp orientation velocity
	const FVector& OrientationVelocityInterp = VelocityInterp.RotateAngleAxis(Character->GetControlRotation().Yaw, FVector(0.f, 0.f, 1.f));
	const FVector& MovementOffset = (-OrientationVelocityInterp / MaxMoveSpeed) * FMath::Max<float>(1.f - ADSMagnitude, 0.6f);
	OffsetLocation += MovementOffset;

	// Add accumulative rotation
	OffsetRotation += AccumulativeRotationInterpInverse;

	// Add movement offset to rotation
	OffsetRotation += FRotator(MovementOffset.Z * 5.f, MovementOffset.Y, MovementOffset.Y * 2.f);
	
	OffsetTransform = Character->FPOffsetTransform * FTransform(OffsetRotation, OffsetLocation, FVector(1.f));
	//UE_LOG(LogTemp, Warning, TEXT("OffsetTransform == %s"), *FTransform(OffsetRotation, OffsetLocation, FVector(1.f)).ToString());
}






void UFPAnimInstance::WeaponChanged_Implementation(AWeapon* NewWeapon, const AWeapon* OldWeapon)
{
	CurrentWeapon = NewWeapon;
	if(!CurrentWeapon) return;
	
	if(UAnimSequence* NewPose = CurrentWeapon->FPAnimPose)
		AnimPose = NewPose;
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UFPAnimInstance::SetIKTransforms);
}

void UFPAnimInstance::SetIKTransforms()
{
	// The sights transform relative to the right hand
	RHandToSightsTransform = CurrentWeapon->GetFPSightsWorldTransform().GetRelativeTransform(Mesh->GetSocketTransform(FName("hand_r")));

	// The aiming point relative to the mesh root. Ends up a bit in front of the camera
	const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	FTransform CameraOffset = Character->GetCamera()->GetComponentTransform().GetRelativeTransform(RootOffset);
	CameraOffset.AddToTranslation(CameraOffset.GetRotation().GetForwardVector() * CurrentWeapon->AimOffset);
	RelativeAimPointTransform = CameraOffset;
}




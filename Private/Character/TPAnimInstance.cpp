// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TPAnimInstance.h"

#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


UTPAnimInstance::UTPAnimInstance()
{
	
}

void UTPAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UTPAnimInstance::NativeUpdateAnimation(float DeltaTime)
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

	// Set old vars for next anim update
	LastAimRotation = Character->GetBaseAimRotation();
	LastVelocity = Character->GetCharacterMovement()->Velocity;
}

void UTPAnimInstance::SetVars_Implementation(const float DeltaTime)
{
	CameraTransform = FTransform(Character->GetBaseAimRotation(), Character->GetCamera()->GetComponentLocation());
	
	/*
	 *	LOCOMOTION VARS
	 */

	MovementDirection = CalculateDirection(Character->GetCharacterMovement()->Velocity, Character->GetBaseAimRotation());

	bIsFalling = Character->GetCharacterMovement()->IsFalling();

	MovementWeaponSwayProgressTime += DeltaTime * (MovementSpeedInterp / MaxMoveSpeed);

	/*
	 *	IK VARS
	 */
	
	ADSMagnitude = Character->ADSValue;
	
	const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	//RelativeCameraTransform = FTransform(Character->GetBaseAimRotation(), Character->GetCamera()->GetComponentLocation()).GetRelativeTransform(RootOffset);
	
	RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);
	

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

void UTPAnimInstance::CalculateWeaponSway(const float DeltaTime)
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


void UTPAnimInstance::Init()
{
	Mesh = Character->GetMesh();

	Character->GetCharacterInventory()->CurrentWeaponChangeDelegate.AddDynamic(this, &UTPAnimInstance::WeaponChanged);

	if(AWeapon* NewWeapon = Character->GetCurrentWeapon())
		WeaponChanged(NewWeapon, nullptr);
}

void UTPAnimInstance::WeaponChanged_Implementation(AWeapon* NewWeapon, const AWeapon* OldWeapon)
{
	CurrentWeapon = NewWeapon;
	if(!CurrentWeapon) return;

	if(UAnimSequence* NewPose = CurrentWeapon->TPAnimPose)
		AnimPose = NewPose;

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UTPAnimInstance::SetIKTransforms);
}

void UTPAnimInstance::SetIKTransforms()
{
	RHandToSightsTransform = CurrentWeapon->GetTPSightsWorldTransform().GetRelativeTransform(Mesh->GetSocketTransform(FName("hand_r")));

	CurrentAimPointOffset = CurrentWeapon->AimOffset;
	
	//RHandToSightsTransform = CurrentWeapon->GetTP_Mesh()->GetSocketTransform(SightsSocketName).GetRelativeTransform(Mesh->GetSocketTransform("hand_r"));

	/*const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	FTransform CameraOffset = Character->GetCamera()->GetComponentTransform().GetRelativeTransform(RootOffset);
	CameraOffset.AddToTranslation(CameraOffset.GetRotation().GetForwardVector() * AimPointOffset);
	RelativeAimPointTransform = CameraOffset;*/

	//const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	//RelativeCameraTransform = Character->GetCamera()->GetComponentTransform().GetRelativeTransform(RootOffset);
}





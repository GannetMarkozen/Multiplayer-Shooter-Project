// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/FPAnimInstance.h"

#include "Character/ShooterCharacter.h"


UFPAnimInstance::UFPAnimInstance()
{
	
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
		if(!Character) return;
	}

	// Aiming value
	ADSMagnitude = Character->ADSMagnitude;

	// UNUSED
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/InspectAbility.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "Objects/InteractInterface.h"

UInspectAbility::UInspectAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	ActivationBlockedTags.AddTag(TAG("Status.State.Interacting"));
}

void UInspectAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if(CHARACTER->IsLocallyControlled())
	{
		const auto& Activate = [=]()->void
		{
			GET_ASC->TryActivateAbility(Spec.Handle);	
		};
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, Activate, InspectFrequency, true);
	}
}

void UInspectAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void UInspectAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	const UCameraComponent* Camera = CHARACTER->GetCamera();
	const FVector& Start = Camera->GetComponentLocation();
	const FVector& End = Start + Camera->GetForwardVector() * InspectDistance;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CHARACTER);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, InspectCollisionChannel, Params);
	//if(Hit.IsValidBlockingHit()) DrawDebugSphere(GetWorld(), Hit.Location, 50.f, 9, FColor::Blue, false, 5.f); // DEBUG
	
	if(Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		if(!IsValid(InspectActor))
		{// If overlapped inspectable actor and none was previously overlapped, inspect
			InspectActor = Hit.GetActor();
			IInteractInterface::Execute_Inspect(InspectActor, CHARACTER);
		}
		else if(Hit.GetActor() != InspectActor)
		{// If overlapping new inspectable actor, end inspect on old inspectable actor and inspect new one
			IInteractInterface::Execute_EndInspect(InspectActor, CHARACTER);
			InspectActor = Hit.GetActor();
			IInteractInterface::Execute_Inspect(InspectActor, CHARACTER);
		}
	}
	else if(IsValid(InspectActor))
	{// If not overlapping inspectable actor and previously was, end inspect
		IInteractInterface::Execute_EndInspect(InspectActor, CHARACTER);
		InspectActor = nullptr;
	}
	/*
	const auto& Reactivate = [=]()->void
	{
		GET_ASC->TryActivateAbility(Handle);
	};
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, Reactivate, InspectFrequency, false);*/
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}



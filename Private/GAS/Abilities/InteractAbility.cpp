// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/InteractAbility.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacter.h"
#include "Objects/InteractInterface.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Abilities/InspectAbility.h"

AActor* GetInspectActor(const UInspectAbility* InspectAbility)
{
	return InspectAbility->InspectActor;
}

UInteractAbility::UInteractAbility()
{
	Input = EAbilityInput::Interact;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Manually will add tag interacting tag to the character
	//ActivationOwnedTags.AddTag(TAG("Status.State.Interacting"));

	ActivationBlockedTags.AddTag(TAG("Status.State.Dead"));
	ActivationBlockedTags.AddTag(TAG("Status.Debuff.Stunned"));
	ActivationBlockedTags.AddTag(TAG("Status.State.Equipping"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Firing"));
	ActivationBlockedTags.AddTag(TAG("WeaponState.Reloading"));
}

void UInteractAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if(ActorInfo->IsNetAuthority() && !ActorInfo->IsLocallyControlled())
	{
		GET_ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericSignalFromClient, Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UInteractAbility::EndInteract);
		GET_ASC->AbilityTargetDataSetDelegate(Spec.Handle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &UInteractAbility::Server_ReceivedTargetData);
	}
}

void UInteractAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	const FVector Start(CHARACTER->GetCamera()->GetComponentLocation());
	const FVector End(Start + CHARACTER->GetCamera()->GetForwardVector() * InteractDistance);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CHARACTER);
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, InteractCollisionChannel, Params);
	//if(Hit.IsValidBlockingHit()) DrawDebugSphere(GetWorld(), Hit.Location, 50.f, 9, FColor::Green, false, 5.f); // DEBUG

	if(Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{// Interact locally
		Interact(Hit.GetActor());

		// Wait release input to end interact and call end ability
		if(UAbilityTask_WaitInputRelease* Task = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
		{
			Task->OnRelease.AddDynamic(this, &UInteractAbility::OnInputReleased);
			Task->Activate();
		}
		else OnInputReleased(0.f);

		// Replicate interactable object to the server and set it there
		const FGameplayAbilityTargetDataHandle Data(new FGameplayAbilityTargetData_ActorArray());
		Data.Data[0].Get()->SetActors({Hit.GetActor()});
		GET_ASC->ServerSetReplicatedTargetData(Handle, ActivationInfo.GetActivationPredictionKey(), Data, FGameplayTag(), ActivationInfo.GetActivationPredictionKey());
	}
	else EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UInteractAbility::Interact_Implementation(AActor* Actor)
{
	GetASC()->AddLooseGameplayTag(InteractingTag);
	IInteractInterface::Execute_Interact(Actor, GetCharacter());
	InteractActor = Actor;
}

void UInteractAbility::EndInteract_Implementation()
{
	GetASC()->RemoveLooseGameplayTag(InteractingTag, 10);
	if(IsValid(InteractActor))
		IInteractInterface::Execute_EndInteract(InteractActor, GetCharacter());
	
	InteractActor = nullptr;
}

void UInteractAbility::Server_ReceivedTargetData(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag)
{
	if(AActor* Actor = Handle.Data[0].Get()->GetActors()[0].Get())
		Interact(Actor);
	
	GetASC()->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UInteractAbility::OnInputReleased(float Time)
{
	EndInteract();
	if(!CurrentActorInfo->IsNetAuthority())
	{// Send event to server that calls EndInteract server-side
		const FPredictionKey& Key = CurrentActivationInfo.GetActivationPredictionKey();
		CurrentActorInfo->AbilitySystemComponent.Get()->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericSignalFromClient, CurrentSpecHandle, Key, Key);
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}




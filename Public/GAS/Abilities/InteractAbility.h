// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "InteractAbility.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UInteractAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UInteractAbility();
protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// Only valid while interacting
	UPROPERTY(BlueprintReadWrite, Category = "Interact Ability")
	class AActor* InteractActor;

	// Called when received valid interactable hit on server
	void Server_ReceivedTargetData(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag);

	// Called locally when released input
	UFUNCTION()
	void OnInputReleased(float Time);

	// The interact implementation. Called on both client and server
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact Ability")
	void Interact(class AActor* Actor);
	virtual void Interact_Implementation(class AActor* Actor);
	
	// Called when input released. Called on both client and server
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact Ability")
	void EndInteract();
	virtual void EndInteract_Implementation();
	
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float InteractDistance = 400.f;
	
	UPROPERTY(EditAnywhere, Category = "Configurations")
	TEnumAsByte<ECollisionChannel> InteractCollisionChannel = ECC_Projectile;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	FGameplayTag InteractingTag = TAG("Status.State.Interacting");
};

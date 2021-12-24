// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "FireWeaponAbility.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UFireWeaponAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UFireWeaponAbility();
protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly)
	bool bCallOnFire = true;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class ULineTraceObject> LineTraceObject;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly)
	int32 NumShots = 1;

	// Called whenever doing line trace
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAS")
	void Server_ReceivedEvent() const;
	virtual void Server_ReceivedEvent_Implementation() const;

	// Called when got a valid blocking hit
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAS")
	void Server_ReceivedTargetData(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag) const;
	virtual void Server_ReceivedTargetData_Implementation(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag) const;
};

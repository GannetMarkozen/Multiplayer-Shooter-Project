// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "HitscanAbility.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UHitscanAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UHitscanAbility();
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

	// The speed at which to play the firing anim
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float PlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	FGameplayTag FiringStateTag;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "GameplayCue"), Category = "Configurations")
	FGameplayTag LocalFiringCue;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "GameplayCue"), Category = "Configurations")
	FGameplayTag NetMulticastFiringCue;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "GameplayCue"), Category = "Configurations")
	FGameplayTag NetMulticastKnockbackCue;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "GameplayCue.Impact"), Category = "Configurations")
	FGameplayTag NetMulticastImpactCue;
	

	// Called whenever doing line trace
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAS")
	void Server_ReceivedEvent() const;
	virtual void Server_ReceivedEvent_Implementation() const;

	// Called when got a valid blocking hit
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GAS")
	void Server_ReceivedTargetData(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag) const;
	virtual void Server_ReceivedTargetData_Implementation(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag) const;
};

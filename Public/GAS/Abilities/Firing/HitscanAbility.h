// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "GAS/Abilities/Weapons/RangedWeapon.h"
#include "HitscanAbility.generated.h"

/**
 * Base class for the hitscan abilities. On ActivateAbility, call Hitscan to fire shot. Other functions can be modified to different behavior.
 */
UCLASS(Abstract)
class MULTIPLAYERSHOOTER_API UHitscanAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	UHitscanAbility();
	
protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual FORCEINLINE void Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo) override
	{
		Super::Client_PredictionFailed_Implementation(ActorInfo);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
	}
	
	// The weapon currently held, also the one that granted this ability most-likely.
	UPROPERTY(BlueprintReadWrite, Category = "Ability|Hit Scan Ability")
	class AHitscanWeapon* CurrentWeapon;

	UPROPERTY(BlueprintReadWrite, Category = "Ability|Hit Scan Ability")
	FTimerHandle RateOfFireTimerHandle;

	UFUNCTION(BlueprintCallable, Category = "Ability|Hit Scan Ability")
	virtual void Hitscan();
	
	// Override this to make your own implementation
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Meta = (AutoCreateRefTerm = "IgnoreActors"), Category = "Ability|Hit Scan Ability")
	void DoLineTrace(FHitResult& Hit, const FVector& Start, const FRotator& Rotation, const TArray<class AActor*>& IgnoreActors);
	virtual void DoLineTrace_Implementation(FHitResult& Hit, const FVector& Location, const FRotator& Rotation, const TArray<class AActor*>& IgnoreActors);

	// Called on both client and server whenever firing
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability|Hit Scan Ability")
	void OnFire(const FGameplayAbilityTargetDataHandle& TargetData, const bool bIsLocal = false);
	virtual void OnFire_Implementation(const FGameplayAbilityTargetDataHandle& TargetData, const bool bIsLocal);

	// The hit scan trace channel
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Projectile;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability|Hit Scan Ability")
	void Server_ReceivedEvent();
	virtual void Server_ReceivedEvent_Implementation();

	// Called when got a valid blocking hit
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability|Hit Scan Ability")
	void Server_ReceivedTargetData(const FGameplayAbilityTargetDataHandle& Handle, FGameplayTag Tag);
	virtual void Server_ReceivedTargetData_Implementation(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag Tag);

	/*
	 *	FIRE-MODE IMPLEMENTATIONS
	 */
	
	// This will only ever be called if the primary fire input has been released while the weapon being held is fully automatic
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability|Hit Scan Ability")
	void FullAuto_PrimaryFireReleased(const float Time);
	virtual void FullAuto_PrimaryFireReleased_Implementation(const float Time);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability|Hit Scan Ability")
	void Burst_Hitscan();
	virtual void Burst_Hitscan_Implementation();

	UPROPERTY(BlueprintReadOnly, Category = "Ability|Hit Scan Ability")
	int32 BurstNumFired = 0;
};

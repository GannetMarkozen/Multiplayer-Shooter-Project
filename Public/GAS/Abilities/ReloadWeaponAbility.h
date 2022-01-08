#pragma once

#include "CoreMinimal.h"
#include "GAS/GASGameplayAbility.h"
#include "ReloadWeaponAbility.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API UReloadWeaponAbility : public UGASGameplayAbility
{
    GENERATED_BODY()
public:
    UReloadWeaponAbility();
protected:
    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void Client_PredictionFailed_Implementation(const FGameplayAbilityActorInfoExtended& ActorInfo) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

    // Speed at which to play the anim montages
    UPROPERTY(EditDefaultsOnly, Category = "Configurations")
    float PlayRate = 1.f;

    // Reload when ammo has been depleted
    UPROPERTY(EditDefaultsOnly, Category = "Configurations")
    bool bReloadOnEnd = true;
};

UCLASS()
class MULTIPLAYERSHOOTER_API UReloadWeaponActivationAbility : public UGASGameplayAbility
{
    GENERATED_BODY()
public:
    UReloadWeaponActivationAbility();
    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

    UFUNCTION()
    void AmmoChanged(int32 Ammo);
};


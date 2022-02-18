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

    // Speed at which to play the reload anim montages
    UPROPERTY(EditDefaultsOnly, Category = "Configurations")
    float PlayRate = 1.f;

    UFUNCTION()
    void Server_SetAmmo(class UGASAbilitySystemComponent* ASC, const FGameplayTag& Tag);

    // Reload when ammo has been depleted
    UPROPERTY(EditDefaultsOnly, Category = "Configurations")
    bool bReloadOnEnd = true;

    UFUNCTION(BlueprintNativeEvent, Category = "GAS|Ability")
    void AmmoChanged(const int32 Ammo);
    virtual void AmmoChanged_Implementation(const int32 Ammo);

    UFUNCTION(BlueprintNativeEvent, Category = "GAS|Ability")
    void TagsChanged(FGameplayTag Tag, int32 Count);
    virtual void TagsChanged_Implementation(FGameplayTag Tag, int32 Count);

    UPROPERTY(BlueprintReadWrite, Category = "GAS|Ability")
    class ARangedWeapon* CurrentWeapon;

    UPROPERTY(EditDefaultsOnly, Meta = (Categories = "WeaponState"), Category = "Configurations")
    FGameplayTag ReloadStateTag = TAG("WeaponState.Reloading");

    UPROPERTY(EditDefaultsOnly, Meta = (Categories = "GameplayCue"), Category = "Configurations")
    FGameplayTag LocalReloadingCue = TAG("GameplayCue.Reload.Local");

    UPROPERTY(EditDefaultsOnly, Meta = (Categories = "GameplayCue"), Category = "Configurations")
    FGameplayTag NetMulticastReloadingCue = TAG("GameplayCue.Reload.NetMulticast");
};

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
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void Client_PredictionFailed_Implementation() override;
   
    UPROPERTY(EditDefaultsOnly, Category = "Configurations")
    TObjectPtr<class UAnimMontage> ReloadMontage;
};


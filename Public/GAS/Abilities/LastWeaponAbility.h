
#pragma once

#include "CoreMinimal.h"
#include "MultiplayerShooter/Public/GAS/GASGameplayAbility.h"
#include "LastWeaponAbility.generated.h"

/**
* 
*/
UCLASS()
class MULTIPLAYERSHOOTER_API ULastWeaponAbility : public UGASGameplayAbility
{
	GENERATED_BODY()
public:
	ULastWeaponAbility();
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

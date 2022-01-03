#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilities/Public/GameplayEffect.h"
#include "DeathEffect.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API UDeathEffect : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UDeathEffect();
};
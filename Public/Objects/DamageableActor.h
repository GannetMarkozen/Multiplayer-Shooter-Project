#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GAS/DamageInterface.h"

#include "DamageableActor.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ADamageableActor : public AActor, public IDamageInterface
{
	GENERATED_BODY()
public:
	ADamageableActor();
protected:
};
#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

// Add interface to any interactable actors
class MULTIPLAYERSHOOTER_API IInteractInterface
{
	GENERATED_BODY()
protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact Interface")
	void Interact(class AShooterCharacter* Instigator);
	virtual FORCEINLINE void Interact_Implementation(class AShooterCharacter* Interactor) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact Interface")
	void EndInteract(class AShooterCharacter* Instigator);
	virtual FORCEINLINE void EndInteract_Implementation(class AShooterCharacter* Interactor) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact Interface")
	void Inspect(class AShooterCharacter* Instigator);
	virtual FORCEINLINE void Inspect_Implementation(class AShooterCharacter* Interactor) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact Interface")
	void EndInspect(class AShooterCharacter* Instigator);
	virtual FORCEINLINE void EndInspect_Implementation(class AShooterCharacter* Interactor) {}
};
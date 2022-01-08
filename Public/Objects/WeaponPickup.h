// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "Objects/ItemPickupBase.h"
#include "WeaponPickup.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AWeaponPickup : public AItemPickupBase
{
	GENERATED_BODY()
public:
	AWeaponPickup();
	
	// Server only. Spawns weapon pickup with the given weapon.
	// Removing weapon from inventory must be done on owner
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "Location, OptionalVelocity"), Category = "Weapon Pickup")
	static class AWeaponPickup* SpawnWeaponPickup(class AWeapon* Weapon, const FVector& Location, const FVector& OptionalVelocity);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "Item Pickup")
	class AWeapon* Weapon;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	virtual void Interact_Implementation(AShooterCharacter* Interactor) override;
	virtual void Inspect_Implementation(AShooterCharacter* Interactor) override;
	virtual void EndInspect_Implementation(AShooterCharacter* Interactor) override;

	// Should only change this variable if placing the weapon in the scene and want to initialize the weapon
	UPROPERTY(EditAnywhere, Category = "Item Pickup|Initialization")
	TSubclassOf<class AWeapon> WeaponClass;
};

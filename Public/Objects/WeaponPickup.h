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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_Weapon, Category = "Item Pickup")
	class AWeapon* Weapon;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	virtual void Interact_Implementation(AShooterCharacter* Interactor) override;
	virtual void Inspect_Implementation(AShooterCharacter* Interactor) override;
	virtual void EndInspect_Implementation(AShooterCharacter* Interactor) override;

	// Init weapon attributes etc (set visibility to false)
	UFUNCTION(BlueprintNativeEvent, Category = "Item Pickup")
	void OnRep_Weapon();
	virtual void OnRep_Weapon_Implementation();

	// The weapon mesh
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> Mesh;

	// Should only change this variable if placing the weapon in the scene and want to initialize the weapon
	UPROPERTY(EditAnywhere, Category = "Item Pickup|Initialization")
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_SkeletalMesh, Category = "Item Pickup")
	class USkeletalMesh* SkeletalMesh;

	UFUNCTION()
	FORCEINLINE void OnRep_SkeletalMesh() { Mesh->SetSkeletalMesh(SkeletalMesh); }

public:
	UFUNCTION(BlueprintCallable, Category = "Item Pickup")
	FORCEINLINE void SetSkeletalMesh(class USkeletalMesh* NewMesh)
	{
		SkeletalMesh = NewMesh;
		OnRep_SkeletalMesh();
	}
};

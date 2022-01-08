// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "InventoryComponent.generated.h"

UINTERFACE(MinimalAPI)
class UInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

class MULTIPLAYERSHOOTER_API IInventoryInterface
{
	GENERATED_BODY()
protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	class UInventoryComponent* GetInventory();
	virtual class UInventoryComponent* GetInventory_Implementation() = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateInventory);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MULTIPLAYERSHOOTER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Items")
	TArray<TSubclassOf<class AWeapon>> DefaultWeapons;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_Weapons, Category = "Items")
	TArray<class AWeapon*> Weapons;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRep_Weapons"), Category = "Items")
	void BP_OnRep_Weapons(const TArray<class AWeapon*>& OldWeapons);

	UFUNCTION()
	virtual FORCEINLINE void OnRep_Weapons(const TArray<class AWeapon*>& OldWeapons) { BP_OnRep_Weapons(OldWeapons); }

	// Whether or not we can add this weapon. Should take into account filters and max size of inventory
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Items")
	bool CanAddItem(const class AWeapon* Weapon);
	virtual FORCEINLINE bool CanAddItem_Implementation(const class AWeapon* Weapon) { return true; }
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Items|Delegates")
	FUpdateInventory UpdateInventoryDelegate;

	// returns num of successfully added items
	UFUNCTION(BlueprintCallable, Category = "Items")
	virtual int32 AddItems(const TArray<class AWeapon*>& NewWeapons);

	// removes item at index and calls update inventory delegate
	UFUNCTION(BlueprintCallable, Category = "Items")
	virtual void RemoveItem(const int32 Index);

	// returns if item was successfully added
	UFUNCTION(BlueprintCallable, Category = "Items")
	FORCEINLINE bool AddItem(class AWeapon* NewWeapon) { return AddItems({NewWeapon}) > 0; }

	UFUNCTION(BlueprintPure, Category = "Items")
	const FORCEINLINE TArray<class AWeapon*>& GetWeapons() const { return Weapons; }
};

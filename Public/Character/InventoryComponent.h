// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "GAS/Abilities/Weapons/Item.h"
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
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Items|Delegates")
	FUpdateInventory UpdateInventoryDelegate;
	
	UFUNCTION(BlueprintCallable, Category = "Items")
	virtual void AddItems(const TArray<class AWeapon*>& NewWeapons);

	UFUNCTION(BlueprintCallable, Category = "Items")
	FORCEINLINE void AddItem(class AWeapon* NewWeapon) { AddItems({NewWeapon}); }

	UFUNCTION(BlueprintPure, Category = "Items")
	const FORCEINLINE TArray<class AWeapon*>& GetWeapons() const { return Weapons; }
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/InventoryComponent.h"
#include "GameplayAbilities/Public/GameplayAbilitySpec.h"
#include "CharacterInventoryComponent.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UCharacterInventoryComponent : public UInventoryComponent
{
	GENERATED_BODY()
public:
	UCharacterInventoryComponent();
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<class AShooterCharacter> OwningCharacter;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TArray<FGameplayAbilitySpecHandle> ActiveWeaponAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 DefaultItemIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory")
	int32 CurrentIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory")
	int32 LastIndex = 0;

	FTimerHandle EquipDelayTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxWeapons = 5;

public:
	virtual int32 AddItems(const TArray<AWeapon*>& NewWeapons) override;
	virtual void RemoveItem(const int32 Index) override;
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCurrentIndex() const { return CurrentIndex; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetLastIndex() const { return LastIndex; }

	// Gets weapon at current index, possibly could be null
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE class AWeapon* GetCurrent() const { return Weapons.IsValidIndex(CurrentIndex) ? Weapons[CurrentIndex] : nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE void SetCurrentIndex(const int32 NewIndex) { LastIndex = CurrentIndex; CurrentIndex = NewIndex; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void GiveAbilities(const class AWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void RemoveAbilities();
};

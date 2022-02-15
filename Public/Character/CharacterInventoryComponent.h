// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/InventoryComponent.h"
#include "GameplayAbilities/Public/GameplayAbilitySpec.h"
#include "CharacterInventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCurrentWeaponChangeDelegate, class AWeapon*, CurrentWeapon, const class AWeapon*, OldWeapon);
/**
 * 
 */

UCLASS()
class MULTIPLAYERSHOOTER_API UCharacterInventoryComponent : public UInventoryComponent
{
	GENERATED_BODY()
public:
	UCharacterInventoryComponent(){}
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	virtual int32 AddItems(const TArray<AWeapon*>& NewWeapons) override;
	virtual void RemoveItem(const int32 Index) override;
	
	// The current weapon index pair
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentWeapon, Category = "Inventory")
	class AWeapon* CurrentWeapon;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Inventory")
	int32 CurrentIndex = 0;

	// Called whenever current weapon changes
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Delegates")
	FCurrentWeaponChangeDelegate CurrentWeaponChangeDelegate;

	/*
	 *	Getters / Setters
	 */

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE class AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	template<typename T>
	FORCEINLINE T* GetCurrentWeapon() const { return Cast<T>(CurrentWeapon); }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCurrentIndex() const { return CurrentIndex; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetLastIndex() const { return LastIndex; }
	
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	class AShooterCharacter* OwningCharacter;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool SetCurrentWeapon(const int32 Index = 0);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE bool SetCurrentWeaponRef(class AWeapon* NewWeapon)
	{
		const int32 Index = Weapons.Find(NewWeapon);
		if(Index != INDEX_NONE)
			return SetCurrentWeapon(Index);
		return false;
	}

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Inventory")
	int32 LastIndex = 0;
	
	UFUNCTION()
	virtual FORCEINLINE void OnRep_CurrentWeapon(const AWeapon* OldWeapon) { CurrentWeaponChanged(OldWeapon, LastIndex); }

	// Called whenever the weapon changes. Called on all instances
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	void CurrentWeaponChanged(const class AWeapon* OldWeapon, const int32 OldIndex);
	virtual void CurrentWeaponChanged_Implementation(const class AWeapon* OldWeapon, const int32 OldIndex);
};
/*
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

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxWeapons = 5;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentWeapon, Meta = (AllowPrivateAccess = "true"), Category = "Inventory")
	class AWeapon* CurrentWeapon;

	UFUNCTION()
	virtual void OnRep_CurrentWeapon(const class AWeapon* OldWeapon);

public:
	// Called whenever changing weapons
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Delegates")
	FCurrentWeaponChangeDelegate CurrentWeaponChangeDelegate;
	
	virtual int32 AddItems(const TArray<AWeapon*>& NewWeapons) override;
	virtual void RemoveItem(const int32 Index) override;
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCurrentIndex() const { return CurrentIndex; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetLastIndex() const { return LastIndex; }

	// Gets weapon at current index, possibly could be null
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE class AWeapon* GetCurrentWeapon() const { return Weapons.IsValidIndex(CurrentIndex) ? Weapons[CurrentIndex] : nullptr; }

	// -1 / INDEX_NONE means equip nothing
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void SetCurrentIndex(const int32 NewIndex = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void GiveAbilities(const class AWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void RemoveAbilities();
};*/

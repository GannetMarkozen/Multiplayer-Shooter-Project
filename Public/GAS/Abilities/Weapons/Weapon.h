// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "GAS/DamageInterface.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FMeshTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector RelativeLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRotator RelativeRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SightsName = "Sights";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName ForegripName = "Foregrip";
};

USTRUCT(BlueprintType)
struct FGameplayAttributeDataHandle
{
	GENERATED_BODY()

	FGameplayAttributeData* Data = nullptr;
	FGameplayAttributeDataHandle(){}
	FGameplayAttributeDataHandle(FGameplayAttributeData* Data) { this->Data = Data; }
	FGameplayAttributeData* operator->() const
	{
		return Data;
	}
};

UCLASS()
class MULTIPLAYERSHOOTER_API UAttributeDataHandleFuncs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Attribute Data")
	static FORCEINLINE bool IsValid(const FGameplayAttributeDataHandle& AttributeDataHandle) { return AttributeDataHandle.Data != nullptr; }
	
	// Only call if you know it is valid
	UFUNCTION(BlueprintPure, Category = "Attribute Data")
	static FORCEINLINE FGameplayAttributeData& Get(const FGameplayAttributeDataHandle& AttributeDataHandle) { return *AttributeDataHandle.Data; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponAmmoUpdated, int32, Ammo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponReserveAmmoUpdated, int32, ReserveAmmo);

DECLARE_MULTICAST_DELEGATE_OneParam(FWeaponAmmoUpdated_Static, int32);
/**
 * 
 */
UCLASS(Abstract)
class MULTIPLAYERSHOOTER_API AWeapon : public AItem, public IDamageCalculationInterface
{
	GENERATED_BODY()
public:
	AWeapon();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FORCEINLINE int32 CalculateDamage_Implementation(const class AActor* Target, const FGameplayEffectSpecHandle& Spec) const override { return BaseDamage; }

	// Idk
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USceneComponent> DefaultScene;

	// First-person weapon mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> FP_Mesh;

	// Third-person weapon mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> TP_Mesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> FP_EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> TP_EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> FP_ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> TP_ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> FP_FireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> TP_FireMontage;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Configurations")
	TArray<TSubclassOf<class UGASGameplayAbility>> WeaponAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "WeaponState"), Category = "Configurations")
	FGameplayTag DelayAmmoReplicationTag = TAG("WeaponState.DelayReplication.Ammo");

	// The base damage that all damage calculations are based off of
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	int32 BaseDamage = 15;

	// This should be lowered if projectile weapon or melee weapon
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	float Range = 50000.f;

	// The max range that the damage calculation uses as its metric
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	float EffectiveRange = 3500.f;

	// The cooldown inbetween shots
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	float RateOfFire = 0.1f;

	// The cooldown during reloading
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	float ReloadDuration = 1.f;

	// The duration of swapping weapons
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	float WeaponSwapDuration = 1.f;

	// Number of shots per execution. Multiple for things like shotguns.
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	int32 NumShots = 1;

	// The type of damage this weapon deals
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Data.DamageType"), Category = "Configurations")
	FGameplayTag DamageType = TAG("Data.DamageType.Bullet");

	// Any extra specifiers for damage calculation like Data.CanHeadshot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Data"), Category = "Configurations")
	FGameplayTagContainer DamageCalculationTags = TAG_CONTAINER("Data.CanHeadshot");
	
public:
	/*
	 *	Weapon aiblities stuff
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void GiveAbilities();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void RemoveAbilities();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "State")
	TArray<FGameplayAbilitySpecHandle> ActiveAbilities;

	/*
	 * Getters
	 */
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class USkeletalMeshComponent* GetFP_Mesh() const { return FP_Mesh; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class USkeletalMeshComponent* GetTP_Mesh() const { return TP_Mesh; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE TArray<class USkeletalMeshComponent*> GetMeshes() const { return {FP_Mesh, TP_Mesh}; }
	
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetFP_EquipMontage() const { return FP_EquipMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetTP_EquipMontage() const { return TP_EquipMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetFP_ReloadMontage() const { return FP_ReloadMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetTP_ReloadMontage() const { return TP_ReloadMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetFP_FireMontage() const { return FP_FireMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetTP_FireMontage() const { return TP_FireMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetRateOfFire() const { return RateOfFire; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetReloadDuration() const { return ReloadDuration; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetWeaponSwapDuration() const { return WeaponSwapDuration; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	const FORCEINLINE TArray<TSubclassOf<class UGASGameplayAbility>>& GetWeaponAbilities() const { return WeaponAbilities; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetBaseDamage() const { return BaseDamage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE int32 GetNumShots() const { return NumShots; }

	// Appends all damage calculation tags for use in damage effect calculation
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE FGameplayTagContainer GetDamageCalculationTags() const { return TAG_CONTAINER({FGameplayTagContainer(DamageType), DamageCalculationTags}); }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class AShooterCharacter* GetCurrentOwner() const { return CurrentOwner; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UInventoryComponent* GetCurrentInventory() const { return CurrentInventory; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetRange() const { return Range; }
	
	friend void CallOnObtained(class AWeapon* Weapon, class UInventoryComponent* Inventory);
	friend void CallOnRemoved(class AWeapon* Weapon, class UInventoryComponent* Inventory);
	friend void CallOnEquipped(class AWeapon* Weapon, class UCharacterInventoryComponent* Inventory);
	friend void CallOnUnEquipped(class AWeapon* Weapon, class UCharacterInventoryComponent* Inventory);
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnObtained(class UInventoryComponent* Inventory);
	virtual void OnObtained_Implementation(class UInventoryComponent* Inventory);

	UFUNCTION(BlueprintNativeEvent)
	void OnRemoved(class UInventoryComponent* Inventory);
	virtual void OnRemoved_Implementation(class UInventoryComponent* Inventory);

	UFUNCTION(BlueprintNativeEvent)
	void OnEquipped(class UCharacterInventoryComponent* Inventory);
	virtual void OnEquipped_Implementation(class UCharacterInventoryComponent* Inventory);

	UFUNCTION(BlueprintNativeEvent)
	void OnUnEquipped(class UCharacterInventoryComponent* Inventory);
	virtual void OnUnEquipped_Implementation(class UCharacterInventoryComponent* Inventory);

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentInventory)
	class UInventoryComponent* CurrentInventory;

	UPROPERTY(BlueprintReadWrite)
	class AShooterCharacter* CurrentOwner;

	UPROPERTY(BlueprintReadWrite)
	class UCharacterInventoryComponent* CurrentCharacterInventory;

	UPROPERTY(BlueprintReadWrite)
	class UGASAbilitySystemComponent* CurrentASC;

	//UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	//void OnRep_CurrentOwner(const class AShooterCharacter* OldOwner);
	//virtual void OnRep_CurrentOwner_Implementation(const class AShooterCharacter* OldOwner);

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnRep_CurrentInventory(const class UInventoryComponent* OldInventory);
	virtual void OnRep_CurrentInventory_Implementation(const class UInventoryComponent* OldInventory);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Ammo, Meta = (EditCondition = "bUseAmmo"))
	int32 Ammo = 0;
	
	// The ammo type to decrement from reserve ammo attribute
	UPROPERTY(EditAnywhere, Category = "Weapon")
	FGameplayAttribute AmmoAttribute;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE void SetVisibility(const bool bIsVisible) const
	{
		TP_Mesh->SetVisibility(bIsVisible);
		FP_Mesh->SetVisibility(bIsVisible);
	}
	
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE int32 GetAmmo() const { return Ammo; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE void SetAmmo(const int32 NewAmmo)
	{
		if(NewAmmo == Ammo) return;
		const int32 OldAmmo = Ammo;
		Ammo = NewAmmo;
		OnRep_Ammo(OldAmmo);
	}

	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FORCEINLINE FGameplayAttribute& GetAmmoAttribute() const { return AmmoAttribute; }

	// Gets reference to ammo attribute data
	UFUNCTION(BlueprintPure, Meta = (DisplayName = "Get Ammo Attribute Data"), Category = "Weapon")
	FGameplayAttributeDataHandle GetAmmoAttributeDataHandle() const
	{
		return FGameplayAttributeDataHandle(GetAmmoAttributeData());
	}

	// Gets ammo attribute data ptr from set
	FGameplayAttributeData* GetAmmoAttributeData() const;
	
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE int32 GetReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE void SetReserveAmmo(const int32 NewReserveAmmo);
	
	// Should be overriden
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanFire() const;
	virtual FORCEINLINE bool CanFire_Implementation() const { return !bUseAmmo || Ammo > 0; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFire();
	virtual FORCEINLINE void OnFire_Implementation();

	UPROPERTY(EditDefaultsOnly)
	bool bUseAmmo = false;

	// Update ammo client-side with the amount server side when mis-predicting
	// ammo consumption. Only call on server, obviously
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE void UpdateClientAmmo()
	{
		if(HasAuthority()) Client_UpdateAmmo(Ammo);
	}

	// Called whenever the amount of ammo changes
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Delegates")
	FWeaponAmmoUpdated AmmoDelegate;

	// Called whenever the amount of reserve ammo changes
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Delegates")
	FWeaponReserveAmmoUpdated ReserveAmmoDelegate;

	FWeaponAmmoUpdated_Static AmmoDelegate_Static;
	FWeaponAmmoUpdated_Static ReserveAmmoDelegate_Static;
	
protected:
	UFUNCTION()
	virtual FORCEINLINE void OnRep_Ammo(const int32& OldAmmo)
	{
		AmmoDelegate.Broadcast(Ammo);
		AmmoDelegate_Static.Broadcast(Ammo);
	}
	
	FORCEINLINE void ReserveAmmoUpdated(const FOnAttributeChangeData& Data)
	{
		ReserveAmmoDelegate.Broadcast(GetReserveAmmo());
		ReserveAmmoDelegate_Static.Broadcast(GetReserveAmmo());
	}

	UFUNCTION(Client, Reliable)
	void Client_UpdateAmmo(const float Value);
	FORCEINLINE void Client_UpdateAmmo_Implementation(const float Value)
	{
		const float OldValue = Ammo;
		Ammo = Value;
		OnRep_Ammo(OldValue);
	}
};  
 
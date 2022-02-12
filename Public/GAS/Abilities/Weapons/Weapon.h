// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Components/TimelineComponent.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "GAS/DamageInterface.h"
#include "GAS/AttributeSets/AmmoAttributeSet.h"
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
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FORCEINLINE int32 CalculateDamage_Implementation(const class AActor* Target, const FGameplayEffectSpecHandle& Spec) const override { return BaseDamage; }

	// Idk
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USceneComponent* DefaultScene;

	// Weapon mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Configurations|Anim")
	class UAnimMontage* EquipMontage;

	UPROPERTY(EditAnywhere, Category = "Configurations|Anim")
	class UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = "Configurations|Anim")
	class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Configurations")
	TArray<TSubclassOf<class UGASGameplayAbility>> WeaponAbilities;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	// The base damage that all damage calculations are based off of
	UPROPERTY(EditAnywhere, Category = "Configurations")
	int32 BaseDamage = 15;

	// This should be lowered if projectile weapon or melee weapon
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float Range = 50000.f;

	// The max range that the damage calculation uses as its metric
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float EffectiveRange = 3500.f;

	// The cooldown inbetween shots
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float RateOfFire = 0.1f;

	// The cooldown during reloading
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float ReloadDuration = 1.f;

	// The duration of swapping weapons
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float WeaponSwapDuration = 1.f;

	// Number of shots per execution. Multiple for things like shotguns.
	UPROPERTY(EditAnywhere, Category = "Configurations")
	int32 NumShots = 1;

	// The type of damage this weapon deals
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Categories = "Data.DamageType"), Category = "Configurations")
	FGameplayTag DamageType = TAG("Data.DamageType.Bullet");

	// Any extra specifiers for damage calculation like Data.CanHeadshot
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Categories = "Data"), Category = "Configurations")
	FGameplayTagContainer DamageCalculationTags = TAG_CONTAINER("Data.CanHeadshot");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	FName WeaponAttachmentSocketName;
	
public:
	/*
	 *	Weapon abilities stuff
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
	FORCEINLINE class USkeletalMeshComponent* GetMesh() const { return Mesh; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetEquipMontage() const { return EquipMontage; }
	
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetFireMontage() const { return FireMontage; }

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
	/*
	 *	Equipping and unequipping stuff
	 */
	UFUNCTION(BlueprintCallable)
	virtual void AttachToWeaponSocket();
	
	UFUNCTION(BlueprintCallable)
	void OnObtained(class UInventoryComponent* Inventory);
	//virtual void OnObtained_Implementation(class UInventoryComponent* Inventory);
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "On Obtained"))
	void BP_OnObtained(class UInventoryComponent* Inventory);

	UFUNCTION(BlueprintCallable)
	void OnRemoved(class UInventoryComponent* Inventory);
	//virtual void OnRemoved_Implementation(class UInventoryComponent* Inventory);
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Removed")
	void BP_OnRemoved(class UInventoryComponent* Inventory);

	UFUNCTION(BlueprintCallable)
	void OnEquipped(class UCharacterInventoryComponent* Inventory);
	//virtual void OnEquipped_Implementation(class UCharacterInventoryComponent* Inventory);
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Equipped")
	void BP_OnEquipped(class UCharacterInventoryComponent* Inventory);

	UFUNCTION(BlueprintCallable)
	void OnUnEquipped(class UCharacterInventoryComponent* Inventory);
	//virtual void OnUnEquipped_Implementation(class UCharacterInventoryComponent* Inventory);
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "On UnEquipped"))
	void BP_OnUnEquipped(class UCharacterInventoryComponent* Inventory);

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentInventory)
	class UInventoryComponent* CurrentInventory;

	UPROPERTY(BlueprintReadWrite)
	class AShooterCharacter* CurrentOwner;

	UPROPERTY(BlueprintReadWrite)
	class UCharacterInventoryComponent* CurrentCharacterInventory;

	UPROPERTY(BlueprintReadWrite)
	class UGASAbilitySystemComponent* CurrentASC;
	
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
		Mesh->SetVisibility(bIsVisible);
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

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE void DecrementAmmo(const int32 Num = 1) { SetAmmo(Ammo - Num); }

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
	virtual FORCEINLINE bool CanFire_Implementation() const { return Ammo > 0; }

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

	/*
	 *	ANIM
	 */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Anim")
	class UAnimSequence* AnimPose;

	// Override the sights transform here. For weapons without sights set this to be the forward direction of the weapon
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Configurations|Anim")
	FTransform GetSightsWorldTransform() const;
	virtual FORCEINLINE FTransform GetSightsWorldTransform_Implementation() const { return Mesh->GetSocketTransform(FName("Sights")); }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Configurations|Anim")
	FTransform GetMuzzleWorldTransform() const;
	virtual FORCEINLINE FTransform GetMuzzleWorldTransform_Implementation() const { return Mesh->GetSocketTransform(FName("Muzzle")); }

	// The offset from the weapon's AnimPose of the weapon when not aiming relative to the sights
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations|Anim")
	FTransform CustomWeaponOffsetTransform;

	// Determines weapon sway responsiveness. 1 is normal, 0 is instantaneous
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations|Anim")
	float WeightScale = 1.f;

	// Determines the distance from the camera to the sights when aiming
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations|Anim")
	float AimOffset = 15.f;
	
protected:
	/*
	 *	Ammo updated
	 */
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


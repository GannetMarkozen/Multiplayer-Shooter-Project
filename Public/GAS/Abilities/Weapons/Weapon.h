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
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
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
	FORCEINLINE float GetRange() const { return Range; }
	
	friend void CallOnObtained(class AWeapon* Weapon, class UInventoryComponent* Inventory);
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnObtained(class UInventoryComponent* Inventory);
	virtual void OnObtained_Implementation(class UInventoryComponent* Inventory);

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentOwner)
	class AShooterCharacter* CurrentOwner;
	
	UPROPERTY(BlueprintReadWrite)
	class UInventoryComponent* CurrentInventory;

	UPROPERTY(BlueprintReadWrite)
	class UCharacterInventoryComponent* CurrentCharacterInventory;

	UPROPERTY(BlueprintReadWrite)
	class UGASAbilitySystemComponent* CurrentASC;

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnRep_CurrentOwner(const class AShooterCharacter* OldOwner);
	virtual void OnRep_CurrentOwner_Implementation(const class AShooterCharacter* OldOwner);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Ammo, Meta = (EditCondition = "bUseAmmo"))
	int32 Ammo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ReserveAmmo, Meta = (EditCondition = "bUseAmmo"))
	int32 ReserveAmmo = 0;
	
public:
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
	FORCEINLINE int32 GetReserveAmmo() const { return ReserveAmmo; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE void SetReserveAmmo(const int32 NewReserveAmmo)
	{
		if(NewReserveAmmo == ReserveAmmo) return;
		const int32 OldReserveAmmo = ReserveAmmo;
		ReserveAmmo = NewReserveAmmo;
		OnRep_ReserveAmmo(OldReserveAmmo);
	}
	
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

	UFUNCTION()
	virtual FORCEINLINE void OnRep_ReserveAmmo(const int32& OldReserveAmmo)
	{
		ReserveAmmoDelegate.Broadcast(ReserveAmmo);
		ReserveAmmoDelegate_Static.Broadcast(ReserveAmmo);
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
 
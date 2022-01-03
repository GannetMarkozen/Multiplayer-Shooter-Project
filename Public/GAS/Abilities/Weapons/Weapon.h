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

	// First-person weapon mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> FP_Mesh;

	// Third-person weapon mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> TP_Mesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> FP_EquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> TP_EquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> FP_ReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> TP_ReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Configurations")
	TArray<TSubclassOf<class UGASGameplayAbility>> WeaponAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	// The base damage that all damage calculations are based off of
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	int32 BaseDamage = 15;

	// This should be lowered if projectile weapon or melee weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	float Range = 50000.f;

	// The max range that the damage calculation uses as its metric
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	float EffectiveRange = 3500.f;

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
	const FORCEINLINE TArray<TSubclassOf<class UGASGameplayAbility>>& GetWeaponAbilities() const { return WeaponAbilities; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetBaseDamage() const { return BaseDamage; }

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
	
public:
	// Should be overriden
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanFire() const;
	virtual FORCEINLINE bool CanFire_Implementation() const { return !bUseAmmo || Ammo > 0; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFire();
	virtual FORCEINLINE void OnFire_Implementation() { Ammo--; }

	UPROPERTY(EditDefaultsOnly)
	bool bUseAmmo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Ammo, Meta = (EditCondition = "bUseAmmo"))
	int32 Ammo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ReserveAmmo, Meta = (EditCondition = "bUseAmmo"))
	int32 ReserveAmmo = 0;
	
protected:
	UFUNCTION()
	virtual FORCEINLINE void OnRep_Ammo(const int32& OldAmmo) {}

	UFUNCTION()
	virtual FORCEINLINE void OnRep_ReserveAmmo(const int32& OldReserveAmmo) {}
};  
 
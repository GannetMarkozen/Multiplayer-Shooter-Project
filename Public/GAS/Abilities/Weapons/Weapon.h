// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "Item.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
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
class MULTIPLAYERSHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TObjectPtr<class USkeletalMesh> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TSubclassOf<class UAnimInstance> AnimInstance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> FP_EquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TObjectPtr<class UAnimMontage> TP_EquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Configurations")
	TArray<TSubclassOf<class UGASGameplayAbility>> WeaponAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	float BaseDamage = 15.f;

	// Make sure to put a Data.DamageType tag in there
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	FGameplayTagContainer DamageCalculationTags = TAG_CONTAINER("Data.DamageType.Bullet");
	
public:
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class USkeletalMesh* GetMesh() const { return Mesh; }

	//UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE const TSubclassOf<class UAnimInstance>& GetAnimInstance() const { return AnimInstance; }
	
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetFP_EquipMontage() const { return FP_EquipMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UAnimMontage* GetTP_EquipMontage() const { return TP_EquipMontage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	const FORCEINLINE TArray<TSubclassOf<class UGASGameplayAbility>>& GetWeaponAbilities() const { return WeaponAbilities; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetBaseDamage() const { return BaseDamage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	const FORCEINLINE FGameplayTagContainer& GetDamageCalculationTags() const { return DamageCalculationTags; }

	friend void CallOnObtained(class AWeapon* Weapon, class UInventoryComponent* Inventory);
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnObtained(class UInventoryComponent* Inventory);
	virtual void OnObtained_Implementation(class UInventoryComponent* Inventory);
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UInventoryComponent> CurrentInventory;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UCharacterInventoryComponent> CurrentCharacterInventory;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class AShooterCharacter> CurrentOwner;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UGASAbilitySystemComponent> CurrentASC;
	
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
 
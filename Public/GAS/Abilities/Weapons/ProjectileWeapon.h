// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "GAS/GASGameplayAbility.h"
#include "ProjectileWeapon.generated.h"

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	SemiAuto		UMETA(DisplayName = "Semi-Auto"),
	FullAuto		UMETA(DisplayName = "Full-Auto"),
	Burst			UMETA(DisplayName = "Burst"),
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponAmmoUpdated, int32, Ammo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponReserveAmmoUpdated, int32, ReserveAmmo);

DECLARE_MULTICAST_DELEGATE_OneParam(FWeaponAmmoUpdated_Static, int32);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	AProjectileWeapon();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_CurrentInventory_Implementation(const UInventoryComponent* OldInventory) override;
	virtual FORCEINLINE bool CanFire_Implementation() const override { return Ammo > 0; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Firing")
	EFireMode FireMode = EFireMode::SemiAuto;

	// The base damage that all damage calculations are based off of
	UPROPERTY(EditAnywhere, Category = "Configurations|Firing")
	int32 BaseDamage = 15;
	
	// Number of shots per execution. Multiple for things like shotguns.
	UPROPERTY(EditAnywhere, Category = "Configurations|Firing")
	int32 NumShots = 1;
	
	// This should be lowered if projectile weapon or melee weapon
	UPROPERTY(EditAnywhere, Category = "Configurations|Firing")
	float Range = 50000.f;

	// The max range that the damage calculation uses as its metric
	UPROPERTY(EditAnywhere, Category = "Configurations|Firing")
	float EffectiveRange = 3500.f;
	
	// The current ammo used in the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Ammo, Meta = (EditCondition = "bUseAmmo"), Category = "Configurations|Ammo")
	int32 Ammo = 0;
	
	// The ammo type to decrement from reserve ammo attribute
	UPROPERTY(EditAnywhere, Category = "Configurations|Firing")
	FGameplayAttribute AmmoAttribute;

	// The cooldown during reloading
	UPROPERTY(EditAnywhere, Category = "Configurations|Firing")
	float ReloadDuration = 1.f;

	// The cooldown inbetween shots
	UPROPERTY(EditAnywhere, Category = "Configurations|Firing")
	float RateOfFire = 0.1f;

	/*
	 *	BURST FIRE SETTINGS
	 */
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true", EditCondition = "FireMode == EFireMode::Burst"), Category = "Configurations|Firing|Burst")
	float BurstRateOfFire = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true", EditCondition = "FireMode == EFireMode::Burst"), Category = "Configurations|Firing|Burst")
	int32 NumShotsPerBurst = 3;

	/*
	 *	COSMETICS
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Cosmetic")
	class UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations|Cosmetic")
	class USoundBase* FiringSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations|Cosmetic")
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations|Cosmetic")
	TSubclassOf<class URecoilInstance> RecoilClass;

public:
	/*
	 *	GETTERS / SETTERS
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon|Firing")
	FORCEINLINE int32 GetAmmo() const { return Ammo; }

	UFUNCTION(BlueprintCallable, Category = "Weapon|Firing")
	FORCEINLINE void SetAmmo(const int32 NewAmmo)
	{
		if(NewAmmo == Ammo) return;
		const int32 OldAmmo = Ammo;
		Ammo = NewAmmo;
		OnRep_Ammo(OldAmmo);
	}

	FORCEINLINE EFireMode GetFireMode() const { return FireMode; }
	FORCEINLINE int32 GetNumShotsPerBurst() const { return NumShotsPerBurst; }
	FORCEINLINE float GetBurstRateOfFire() const { return BurstRateOfFire; }
	
	UFUNCTION(BlueprintCallable, Category = "Weapon|Firing")
	FORCEINLINE void DecrementAmmo(const int32 Num = 1) { SetAmmo(Ammo - Num); }

	UFUNCTION(BlueprintPure, Category = "Getters")
	const FORCEINLINE FGameplayAttribute& GetAmmoAttribute() const { return AmmoAttribute; }
	
	
	// Gets ammo attribute data ptr from set
	FGameplayAttributeData* GetAmmoAttributeData() const;
	
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE int32 GetReserveAmmo() const
	{
		if(const FGameplayAttributeData* ReserveAmmoAttribute = GetAmmoAttributeData())
			return ReserveAmmoAttribute->GetCurrentValue();
		return INDEX_NONE;
	}
	
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetRateOfFire() const { return RateOfFire; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetReloadDuration() const { return ReloadDuration; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	const FORCEINLINE TArray<TSubclassOf<class UGASGameplayAbility>>& GetWeaponAbilities() const { return WeaponAbilities; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetBaseDamage() const { return BaseDamage; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE int32 GetNumShots() const { return NumShots; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetReserveAmmo(const int32 NewReserveAmmo);

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetRange() const { return Range; }

	FORCEINLINE class UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	
	/*
	*	Ammo updated
	*/

	// Update ammo client-side with the amount server side when mis-predicting
	// ammo consumption. Only call on server, obviously
	UFUNCTION(BlueprintCallable, Category = "Weapon|Firing")
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

protected:
	/*
	 *	COSMETIC FIRING EFFECT
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void Multi_CallOnFireWeapon(const FGameplayAbilityTargetDataHandle& TargetData);
	FORCEINLINE void Multi_CallOnFireWeapon_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
	{
		OnFireWeapon(TargetData);
	}

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_CallOnFireWeapon_SkipLocal(const FGameplayAbilityTargetDataHandle& TargetData);
	FORCEINLINE void Multi_CallOnFireWeapon_SkipLocal_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
	{
		if(!IsLocallyControlledOwner()) OnFireWeapon(TargetData);
	}

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon|Cosmetic")
	FORCEINLINE void Multi_OnFireWeapon(const FGameplayAbilityTargetDataHandle& TargetData, const bool bSkipLocal = false)
	{
		if(!HasAuthority())
		{
			UE_LOG(LogTemp, Error, TEXT("Tried calling %s without authority"), *FString(__FUNCTION__));
			return;
		}
		
		if(bSkipLocal)
		{
			Multi_CallOnFireWeapon_SkipLocal(TargetData);
			Multi_CallOnFireWeapon_SkipLocal_Implementation(TargetData);
		}
		else
		{
			Multi_CallOnFireWeapon(TargetData);
			Multi_CallOnFireWeapon_Implementation(TargetData);
		}
	}

	/*
	 *	FIRING LOGIC
	 */

	// Plays cosmetic firing effect
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Cosmetic")
	void OnFireWeapon(const FGameplayAbilityTargetDataHandle& TargetData);
	virtual void OnFireWeapon_Implementation(const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Cosmetic")
	void OnFireWeaponEnd();
	virtual void OnFireWeaponEnd_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Weapon|Cosmetic")
	FVector2D CalculateSpread() const;
	virtual FORCEINLINE FVector2D CalculateSpread_Implementation() const { return FVector2D::ZeroVector; }
};

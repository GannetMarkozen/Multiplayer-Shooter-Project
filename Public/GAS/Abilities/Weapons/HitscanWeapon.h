// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "Character/ShooterCharacter.h"
#include "GAS/GASGameplayAbility.h"
#include "HitscanWeapon.generated.h"

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	SemiAuto		UMETA(DisplayName = "Semi-Auto"),
	FullAuto		UMETA(DisplayName = "Full-Auto"),
	Burst			UMETA(DisplayName = "Burst"),
};

/** Contains the parameters and calculation requirements for a hitscan weapon
 * including damage falloff and line trace ability
 */ 
UCLASS(Abstract)
class MULTIPLAYERSHOOTER_API AHitscanWeapon : public ARangedWeapon
{
	GENERATED_BODY()
public:
	AHitscanWeapon();
	
protected:
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	virtual void SetupInputBindings() override;
	virtual void RemoveInputBindings() override;
	virtual int32 CalculateDamage_Implementation(const class AActor* Target, const FGameplayEffectSpecHandle& Spec) const override;
	virtual void OnFireWeapon_Implementation(const FGameplayAbilityTargetDataHandle& TargetData) override;
	virtual FORCEINLINE void OnUnequipped(UCharacterInventoryComponent* Inventory) override
	{
		Super::OnUnequipped(Inventory);
		StopFiring();
	}

	UPROPERTY(EditAnywhere, Instanced, Meta = (DisplayName = "Firing Mode"), Category = "Configurations|Firing")
	class UHitscanFiringObject* HitscanObject;

	UPROPERTY(EditAnywhere, Instanced, Category = "Configurations|Firing")
	TArray<class UHitscanFiringObject*> RandomObjects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Firing")
	EFireMode FireMode = EFireMode::SemiAuto;

	// Checked against source (CurrentOwner). If any of these exist we can not fire.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Firing")
	FGameplayTagContainer FiringBlockedTags;

	// Number of shots per execution. Multiple for things like shotguns.
	UPROPERTY(EditAnywhere, Category = "Configurations|Firing")
	int32 NumShots = 1;

	/*
	*	BURST FIRE SETTINGS
	*/
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true", EditCondition = "FireMode == EFireMode::Burst"), Category = "Configurations|Firing|Burst")
	float BurstRateOfFire = 0.6f;

	int32 CurrentNumBurst = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true", EditCondition = "FireMode == EFireMode::Burst"), Category = "Configurations|Firing|Burst")
	int32 NumShotsPerBurst = 3;

	/*
	 *	COSMETIC EFFECTS
	 */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Cosmetic")
	class UParticleSystem* BulletTracer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Cosmetic")
	class USoundBase* FinalShotSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Cosmetic")
	FVector2D FiringSpread;

	/*
	 *	FIRING LOGIC
	 */

	// Curve float determining the damage at a given range in terms of a multiplier to the BaseDamage, 1 being full BaseDamage. Range of 1 == max range on the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations|Damage Calculation")
	class UCurveFloat* DamageFalloffCurve;

	// Whether or not we should fire the weapon, called locally and server
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Hitscan")
	bool CanFireWeapon() const;
	virtual FORCEINLINE bool CanFireWeapon_Implementation() const;
	
public:
	static const FGameplayTag FiringStateTag;

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Hitscan")
	float CalculateFiringSpreadMagnitude() const;
	virtual FORCEINLINE float CalculateFiringSpreadMagnitude_Implementation() const { return 1.f; }
	
	/*
	 *	GETTERS
	 */
	
	FORCEINLINE EFireMode GetFireMode() const { return FireMode; }
	FORCEINLINE int32 GetNumShotsPerBurst() const { return NumShotsPerBurst; }
	FORCEINLINE float GetBurstRateOfFire() const { return BurstRateOfFire; }
	FORCEINLINE const FVector2D& GetFiringSpread() const { return FiringSpread; }
	FORCEINLINE int32 GetNumShots() const { return NumShots; }

	/*
	 *	HITSCAN LOGIC
	 */

	FTimerHandle FiringTimerHandle;
	
	// Called whenever PrimaryFire is pressed
	UFUNCTION(BlueprintCallable, Category = "Hitscan")
	virtual void StartFiring();

	// Called whenever PrimaryFire is released
	UFUNCTION(BlueprintCallable, Category = "Hitscan")
	virtual void StopFiring();

	// Plays the hitscan damage / visual-effect logic
	UFUNCTION(BlueprintCallable, Category = "Hitscan")
	virtual void Hitscan();

	FORCEINLINE void Burst_Hitscan()
	{
		Hitscan();
		if(++CurrentNumBurst > NumShotsPerBurst)
		{
			CurrentNumBurst = 0;
			GetWorld()->GetTimerManager().ClearTimer(FiringTimerHandle);
		}
	}

	FORCEINLINE void FullAuto_Hitscan()
	{
		Hitscan();
		CurrentASC->AddLooseGameplayTagForDurationSingle(FiringStateTag, RateOfFire);
	}

protected:
	UFUNCTION(Server, Reliable)
	virtual void Server_Hitscan(const FGameplayAbilityTargetDataHandle& DataHandle);
	
};
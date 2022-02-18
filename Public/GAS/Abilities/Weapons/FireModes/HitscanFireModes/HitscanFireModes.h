#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Weapons/FireModes/FiringObject.h"
#include "Character/ShooterCharacter.h"
#include "GAS/Abilities/Weapons/HitscanWeapon.h"
#include "GAS/Abilities/Weapons/RecoilInstance.h"
#include "HitscanFireModes.generated.h"


UCLASS(Abstract)
class MULTIPLAYERSHOOTER_API UHitscanFiringObject : public UFiringObject
{
	GENERATED_BODY()
public:
	UHitscanFiringObject()
	{
		SourceBlockedTags.AppendTags(TAG_CONTAINER({"Status.State.Dead", "Status.Debuff.Stunned", "WeaponState.Reloading", "WeaponState.Firing"}));
	}
	
	// Tags required to fire
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	FGameplayTagContainer SourceRequiredTags;

	// Tags that must not be present to fire
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	FGameplayTagContainer SourceBlockedTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	float RateOfFire = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	class USoundBase* FiringSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	class UParticleSystem* BulletTracer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	class UNiagaraSystem* NiagaraBulletTracer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	TSubclassOf<class URecoilInstance> RecoilClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "State")
	class AHitscanWeapon* Weapon;

	UFUNCTION(BlueprintPure, Category = "Weapon Firing")
	FORCEINLINE class UGASAbilitySystemComponent* GetASC() const { return Weapon->GetCurrentASC(); }

	UFUNCTION(BlueprintPure, Category = "Weapon Firing")
	FORCEINLINE class AShooterCharacter* GetCharacter() const { return Weapon->GetCurrentOwner(); }

	virtual FORCEINLINE void StartFiring_Implementation() override
	{
		if(!Weapon)
		{
			Weapon = GetTypedOuter<AHitscanWeapon>();
			Initialize();
		}
	}

	virtual FORCEINLINE bool CanFire_Implementation() const override
	{
		return Weapon && Weapon->CanFire() && GetASC()->HasAllMatchingGameplayTags(SourceRequiredTags) && !GetASC()->HasAnyMatchingGameplayTags(SourceBlockedTags);
	}

	// Called once when first firing the weapon
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon Firing")
	void Initialize();
	virtual void Initialize_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Weapon Firing")
	bool CanHitscan() const;
	virtual FORCEINLINE bool CanHitscan_Implementation() const { return CanFire(); }

	UFUNCTION(BlueprintCallable, Category = "Weapon Firing")
	virtual void Hitscan();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon Firing")
	void OnFireWeapon(const FGameplayEffectContextHandle& Context);
	virtual void OnFireWeapon_Implementation(const FGameplayEffectContextHandle& Context);

	// Skips local
	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable, Category = "Weapon Firing")
	void Multi_CallOnFireWeapon(const FGameplayEffectContextHandle& Context);
	virtual FORCEINLINE void Multi_CallOnFireWeapon_Implementation(const FGameplayEffectContextHandle& Context)
	{
		if(!Weapon) Weapon = GetTypedOuter<AHitscanWeapon>();
		if(GetCharacter()->IsLocallyControlled()) return;
		OnFireWeapon(Context);
	}

	

protected:
	// Called right before RPC. Modify target data in here to send custom params
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon Firing")
	void LocalModifyContext(FGameplayEffectContextHandle& Context) const;
	virtual FORCEINLINE void LocalModifyContext_Implementation(FGameplayEffectContextHandle& Context) const {}
	
	UFUNCTION(Server, Reliable)
	void Server_Hitscan(const FGameplayEffectContextHandle& Context);
	virtual void Server_Hitscan_Implementation(const FGameplayEffectContextHandle& Context);

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "Server Hitscan"), Category = "Weapon Firing")
	void BP_Server_Hitscan(const FGameplayEffectContextHandle& Context);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon Firing")
	void SpawnFiringSound(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform);
	virtual FORCEINLINE void SpawnFiringSound_Implementation(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform)
	{
		if(FiringSound)
			UGameplayStatics::SpawnSoundAttached(FiringSound, Weapon->GetMesh(), FName("Muzzle"),
				MuzzleWorldTransform.GetLocation(), MuzzleWorldTransform.Rotator(), EAttachLocation::KeepWorldPosition);
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon Firing")
	void SpawnMuzzleFlash(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform);
	virtual FORCEINLINE void SpawnMuzzleFlash_Implementation(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform)
	{
		if(MuzzleFlash)
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, Weapon->GetMesh(), FName("Muzzle"),
				MuzzleWorldTransform.GetLocation(), MuzzleWorldTransform.Rotator(), FVector(1.f), EAttachLocation::KeepWorldPosition);
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon Firing")
	void SpawnRecoilInstance(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform);
	virtual FORCEINLINE void SpawnRecoilInstance_Implementation(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform)
	{
		if(RecoilClass)
			URecoilInstance::AddRecoilInstance(GetCharacter(), RecoilClass);
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon Firing")
	void SpawnBulletTracers(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform);
	virtual void SpawnBulletTracers_Implementation(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform);
};

/*
 *	
 *
 *
 *	SEMI-AUTO
 *
 *
 *
 *
 *
 */



// Object for handling semi-automatic firing on AHitscanWeapon
UCLASS(meta = (DisplayName = "Semi-Auto"))
class MULTIPLAYERSHOOTER_API USemiAutoFiringObject : public UHitscanFiringObject
{
	GENERATED_BODY()
public:
	virtual void StartFiring_Implementation() override;
};

/*
 *
 *
 *
 *	BURST-FIRE
 *
 *
 *
 *
 */

// Object for handling burst-fire firing on AHitscanWeapon
UCLASS(meta = (DisplayName = "Burst-Fire"))
class MULTIPLAYERSHOOTER_API UBurstFiringObject : public UHitscanFiringObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "FiringSound"), Category = "Configurations")
	class USoundBase* LastFiringSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayAfter = "RateOfFire"), Category = "Configurations")
	float BurstRateOfFire = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	int32 NumShotsPerBurst = 3;

	// This tag can really be anything but we 
	// some kind of indicator as to when the final shot has been fired
	static const FGameplayTag LastShotTag;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentNumShotsPerBurst = 0;

	FTimerHandle FiringTimerHandle;

	virtual void Initialize_Implementation() override
	{
		Weapon->UnequippedDelegate.AddDynamic(this, &ThisClass::OnUnequipped);
	}

	virtual bool CanHitscan_Implementation() const override
	{
		if(!Weapon) return false;
		FGameplayTagContainer OwnedTags;
		GetASC()->GetOwnedGameplayTags(OwnedTags);
		OwnedTags.RemoveTag(FiringStateTag);
		return Weapon->CanFire() && OwnedTags.HasAll(SourceRequiredTags) && !OwnedTags.HasAny(SourceBlockedTags);//GetASC()->HasAllMatchingGameplayTags(SourceRequiredTags) && !GetASC()->HasAnyMatchingGameplayTags(SourceBlockedTags);
	}

	virtual FORCEINLINE void LocalModifyContext_Implementation(FGameplayEffectContextHandle& Context) const override
	{
		if(CurrentNumShotsPerBurst >= NumShotsPerBurst)
			((FGameplayEffectContextExtended*)Context.Get())->CustomTags.AddTag(LastShotTag);
	}

	virtual FORCEINLINE void SpawnFiringSound_Implementation(const FGameplayEffectContextHandle& Context, const FTransform& MuzzleWorldTransform) override
	{
		if(LastFiringSound && Extend(Context)->CustomTags.HasTagExact(LastShotTag))
		{
			UGameplayStatics::SpawnSoundAttached(LastFiringSound, Weapon->GetMesh(), FName("Muzzle"),
				MuzzleWorldTransform.GetLocation(), MuzzleWorldTransform.Rotator(), EAttachLocation::KeepWorldPosition);
		}
		else if(FiringSound)
		{
			UGameplayStatics::SpawnSoundAttached(FiringSound, Weapon->GetMesh(), FName("Muzzle"),
				MuzzleWorldTransform.GetLocation(), MuzzleWorldTransform.Rotator(), EAttachLocation::KeepWorldPosition);
		}
	}

	/*virtual void OnFireWeapon_Implementation(const FGameplayEffectContextHandle& Context) override
	{
		const FTransform& MuzzleWorldTransform = Weapon->GetMuzzleWorldTransform();
		SpawnFiringSound(((FGameplayEffectContextExtended*)Context.Get())->CustomTags.HasTagExact(LastShotTag) ? FiringSound : LastFiringSound, MuzzleWorldTransform);
		SpawnMuzzleFlash(MuzzleFlash, MuzzleWorldTransform);	
		SpawnRecoilInstance(RecoilClass);
		SpawnBulletTracers(BulletTracer, Context, MuzzleWorldTransform);
	}*/

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon Firing")
	void OnUnequipped(class UInventoryComponent* OldInventory);
	virtual FORCEINLINE void OnUnequipped_Implementation(class UInventoryComponent* OldInventory)
	{// Stop firing if switched weapons mid-firing
		GetWorld()->GetTimerManager().ClearTimer(FiringTimerHandle);
	}

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon Firing")
	void Burst_Hitscan();
	virtual void Burst_Hitscan_Implementation()
	{
		if(CurrentNumShotsPerBurst++ >= NumShotsPerBurst)
		{
			GetWorld()->GetTimerManager().ClearTimer(FiringTimerHandle);
			CurrentNumShotsPerBurst = 0;
			return;
		}
		Hitscan();
	}
	
	virtual void StartFiring_Implementation() override;
};

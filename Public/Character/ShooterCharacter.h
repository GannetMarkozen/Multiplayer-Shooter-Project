// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/TimelineComponent.h"
#include "GAS/DamageInterface.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"

#include "ShooterCharacter.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChangedWeapons, class AWeapon*, NewWeapon, const class AWeapon*, OldWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLandedMulticast, class AShooterCharacter*, Character, const FHitResult&, Hit);

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterCharacter : public ACharacter, public IAbilitySystemInterface, public IInventoryInterface, public IDamageInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();

protected:
	virtual void BeginPlay() override;
	
public:	
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual FORCEINLINE class UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	virtual FORCEINLINE class UInventoryComponent* GetInventory_Implementation() override { return Inventory; }

protected:
	// Only visible or exists locally. 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"), Category = "Components")
	class USkeletalMeshComponent* ClientMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"), Category = "Components")
	class USpringArmComponent* CameraSpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Components")
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Components")
	class UGASAbilitySystemComponent* ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<class UCharacterInventoryComponent> Inventory;

	virtual void MoveForward(float Value);
	virtual void MoveRight(float Value);
	virtual void LookUp(float Value);
	virtual void Turn(float Value);

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	float Sensitivity = 0.8f;
	
	virtual void InitializeAbilities();
	virtual void InitializeAttributes();
	
	UFUNCTION(BlueprintNativeEvent)
	void PostInitializeAbilities();
	virtual FORCEINLINE void PostInitializeAbilities_Implementation() {}

	// Default Abilities tgo be applied to the owner on BeginPlay with some extra information
	UPROPERTY(EditDefaultsOnly, Category = "Configurations|GAS")
	TArray<TSubclassOf<class UGASGameplayAbility>> DefaultAbilities;

	// Default Effects applied to owner on BeginPlay
	UPROPERTY(EditDefaultsOnly, Category = "Configurations|GAS")
	TArray<TSubclassOf<class UGameplayEffect>> DefaultEffects;

	// The class this shooter character is
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "Class"), Category = "Configurations")
	FGameplayTag ClassTag = TAG("Class.None");

	FTimerHandle DelayEquipHandle;
	
public:
	/*
	 *	ATTRIBUTE SETS
	 */
	
	UPROPERTY(BlueprintReadWrite)
	class UCharacterAttributeSet* CharacterSet;

	UPROPERTY(BlueprintReadWrite)
	class UAmmoAttributeSet* AmmoSet;

	/*
	 *	GETTERS
	 */
	
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UGASAbilitySystemComponent* GetASC() const { return ASC; }

	// The class this shooter character is (gameplay class)
	UFUNCTION(BlueprintPure, Category = "Getters")
	const FORCEINLINE FGameplayTag& GetClassTag() const { return ClassTag; }
	
	UFUNCTION(BlueprintPure, Category = "Getters")
	const FORCEINLINE TArray<TSubclassOf<class UGASGameplayAbility>>& GetDefaultAbilities() const { return DefaultAbilities; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UCharacterInventoryComponent* GetCharacterInventory() const { return Inventory; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	// Immediately kills the player
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "OptionalSpec"), Category = "Character")
	virtual void Die(const FGameplayEffectSpecHandle& OptionalSpec);

protected:
	/*
	 *	HEALTH / DEATH IMPLEMENTATIONS
	 */
	
	// Called when health changed
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	// The spec is the killing effect spec handle
	UFUNCTION(BlueprintNativeEvent, Category = "Character")
	void Server_Death(const float Magnitude, const FGameplayEffectSpecHandle& Spec);
	virtual void Server_Death_Implementation(const float Magnitude, const FGameplayEffectSpecHandle& Spec);

	// Called upon death on all instances
	UFUNCTION(BlueprintNativeEvent, Category = "Character")
	void Death(const float Magnitude, const FGameplayEffectSpecHandle& Spec);
	virtual void Death_Implementation(const float Magnitude, const FGameplayEffectSpecHandle& Spec);

	UFUNCTION(NetMulticast, Reliable)
	void Multi_Death(const float Magnitude, const FGameplayEffectSpec& Spec);
	FORCEINLINE void Multi_Death_Implementation(const float Magnitude, const FGameplayEffectSpec& Spec)
	{
		Death(Magnitude, FGameplayEffectSpecHandle(new FGameplayEffectSpec(Spec)));
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void Ragdoll(const float Magnitude, const FGameplayEffectSpecHandle& OptionalSpec);
	virtual void Ragdoll_Implementation(const float Magnitude, const FGameplayEffectSpecHandle& Spec);

	UFUNCTION(Server, Reliable)
	void Server_Die(const FGameplayEffectSpec& OptionalSpec);
	FORCEINLINE void Server_Die_Implementation(const FGameplayEffectSpec& OptionalSpec)
	{
		Die(OptionalSpec.Def ? FGameplayEffectSpecHandle(new FGameplayEffectSpec(OptionalSpec)) : FGameplayEffectSpecHandle());
	}
	
	UPROPERTY(EditDefaultsOnly, Category = "Configurations|GAS")
	TSubclassOf<class UGameplayEffect> DeathEffectClass;
	
public:
	/*
	 *	HUD
	 */
	
	// Update HUD
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Meta = (AutoCreateRefTerm = "ReserveAmmoText"), Category = "HUD")
	void SetReserveAmmoText(const FText& ReserveAmmoText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Meta = (AutoCreateRefTerm = "AmmoText"), Category = "HUD")
	void SetAmmoText(const FText& AmmoText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Meta = (AutoCreateRefTerm = "InspectText"), Category = "HUD")
	void SetInspectText(const FText& InspectText);
	
	UFUNCTION(BlueprintPure, Category = "Character")
	FORCEINLINE class AWeapon* GetCurrentWeapon() const
	{
		return Inventory->GetCurrentWeapon();
	}


	/*
	 *	Procedural FP weapon animation stuff
	 */
	
	// The amount we are currently aiming. From 0 - 1
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "Anim")
	float ADSValue = 0.f;

	// FP arms mesh ik offset
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Anim")
	FTransform FPOffsetTransform;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations|Anim")
	class UDataTable* ItemMeshDataTable;

	// Called whenever landed and multicasted
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FLandedMulticast LandedMultiDelegate;

protected:
	virtual FORCEINLINE void Landed(const FHitResult& Hit) override
	{
		Super::Landed(Hit);
		LandedMultiDelegate.Broadcast(this, Hit);
		if(HasAuthority() && LandedMultiDelegate.IsBound())
			Multi_Landed(Hit);
	}
	
	UFUNCTION(NetMulticast, Reliable)
	void Multi_Landed(const FHitResult& Hit);
	virtual FORCEINLINE void Multi_Landed_Implementation(const FHitResult& Hit)
	{
		if(!IsLocallyControlled())
			LandedMultiDelegate.Broadcast(this, Hit);
	}
};

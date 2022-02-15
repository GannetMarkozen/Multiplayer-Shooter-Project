// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "InputBinding.h"
#include "Item.h"
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

enum EInputBindModType
{
	Bind, Remove
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
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FORCEINLINE int32 CalculateDamage_Implementation(const class AActor* Target, const FGameplayEffectSpecHandle& Spec) const override { return 0.f; }

	// Idk
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class USceneComponent* DefaultScene;

	// Weapon mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"), Category = "Components")
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = "true"), Category = "Configurations|Cosmetic")
	class UAnimMontage* EquipMontage;

	// The blueprint binds. Do not set these in C++, use the SetupBinds event instead
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	TArray<FInputBindingInfo> Binds;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Configurations")
	TArray<TSubclassOf<class UGASGameplayAbility>> WeaponAbilities;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	// The duration of swapping weapons
	UPROPERTY(EditAnywhere, Category = "Configurations")
	float WeaponSwapDuration = 1.f;

	// The type of damage this weapon deals
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Categories = "Data.DamageType"), Category = "Configurations|Damage Calculation")
	FGameplayTag DamageType = TAG("Data.DamageType.Bullet");

	// Any extra specifiers for damage calculation like Data.CanHeadshot
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Categories = "Data"), Category = "Configurations|Damage Calculation")
	FGameplayTagContainer DamageCalculationTags = TAG_CONTAINER("Data.CanHeadshot");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configurations")
	FName WeaponAttachmentSocketName;
	
public:
	/*
	 *	BINDS
	 */

	// Called whenever equipped. Bind your inputs here using AWeapon::SetupBind
	UFUNCTION(BlueprintCallable, Category = "Binds")
	virtual void SetupInputBindings();

	UFUNCTION(BlueprintCallable, Category = "Binds")
	virtual void RemoveInputBindings();
	
	template<typename UserClass, typename ReturnType, typename... VarTypes, typename EnumType>
	FORCEINLINE void SetupBind(UserClass* UserObject, ReturnType(UserClass::* InMemFuncPtr)(VarTypes...), const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed)
	{
		UInputBinding::BindInputUObject(CurrentOwner, UserObject, InMemFuncPtr, EnumValue, InputEvent);
	}

	template<typename UserClass, typename ReturnType, typename... VarTypes>
	FORCEINLINE void SetupBind(UserClass* UserObject, ReturnType(UserClass::* InMemFuncPtr)(VarTypes...))
	{
		UInputBinding::BindInputUObject(CurrentOwner, UserObject, InMemFuncPtr, EInputBinding::PrimaryFire, IE_Pressed);
	}
	
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
	
	FORCEINLINE class USkeletalMeshComponent* GetMesh() const { return Mesh; }
	FORCEINLINE class UAnimMontage* GetEquipMontage() const { return EquipMontage; }

	// Appends all damage calculation tags for use in damage effect calculation
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE FGameplayTagContainer GetDamageCalculationTags() const { return TAG_CONTAINER({FGameplayTagContainer(DamageType), DamageCalculationTags}); }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class AShooterCharacter* GetCurrentOwner() const { return CurrentOwner; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UInventoryComponent* GetCurrentInventory() const { return CurrentInventory; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE float GetWeaponSwapDuration() const { return WeaponSwapDuration; }
	
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
	void OnUnequipped(class UCharacterInventoryComponent* Inventory);
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
	
public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE void SetVisibility(const bool bIsVisible) const
	{
		Mesh->SetVisibility(bIsVisible);
	}
	
	
	
	// Should be overriden
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanFire() const;
	virtual FORCEINLINE bool CanFire_Implementation() const { return true; }

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
	 *	HELPERS
	 */
	UFUNCTION(BlueprintPure)
	bool IsLocallyControlledOwner() const;

	template<typename UserClass, typename ReturnType, typename... ParamTypes, typename EnumType>
	FORCEINLINE void Internal_AddBindingDynamic(ReturnType(UserClass::* MemFuncPtr)(ParamTypes...), const FName& FuncName, const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent)
	{
		check(MemFuncPtr != nullptr);
		Binds.Add(FInputBindingInfo(EnumValue, InputEvent, FuncName));
	}
};

// Adds the input binding from a member function pointer. Mark the function as a UFUNCTION. Should not use this, use CREATE_BIND in AWeapon::SetBindings
#define AddBinding(MemFunc, Input, Event) Internal_AddBindingDynamic(MemFunc, STATIC_FUNCTION_FNAME(TEXT(#MemFunc)), Input, Event)

// Call this inside AWeapon::SetBindings to create a binding on equipped that gets removed on unequipped. No UFUNCTION declaration required
#define DECLARE_WEAPON_BIND(MemFunc, Input, Event)\
	{ (BindMod == EInputBindModType::Bind) ? UInputBinding::BindInputUObject(CurrentOwner, this, MemFunc, Input, Event) :\
	UInputBinding::RemoveInputUObject(CurrentOwner, this, Input, Event); }


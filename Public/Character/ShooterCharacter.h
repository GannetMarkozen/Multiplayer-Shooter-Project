// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GAS/GASAbilitySystemComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "GAS/DamageInterface.h"

#include "ShooterCharacter.generated.h"


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
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual FORCEINLINE class UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	virtual FORCEINLINE class UInventoryComponent* GetInventory_Implementation() override { return Inventory; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<class UCameraComponent> Camera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> FP_Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<class UGASAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<class UGASAttributeSet> Attributes;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Components")
	TObjectPtr<class UCharacterInventoryComponent> Inventory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	TObjectPtr<class UDataTable> ItemMeshDataTable;

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

	FTimerHandle DelayEquipHandle;
	
public:
	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UGASAbilitySystemComponent* GetASC() const { return ASC; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class USkeletalMeshComponent* GetFP_Mesh() const { return FP_Mesh; }

	// Must be set in BP
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Getters")
	class USkeletalMeshComponent* GetFP_ItemMesh() const;
	FORCEINLINE class USkeletalMeshComponent* GetFP_ItemMesh_Implementation() const { return nullptr; }

	// Must be set in BP
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Getters")
	class USkeletalMeshComponent* GetTP_ItemMesh() const;
	FORCEINLINE class USkeletalMeshComponent* GetTP_ItemMesh_Implementation() const { return nullptr; }
	
	UFUNCTION(BlueprintPure, Category = "Getters")
	const FORCEINLINE TArray<TSubclassOf<class UGASGameplayAbility>>& GetDefaultAbilities() const { return DefaultAbilities; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UCharacterInventoryComponent* GetCharacterInventory() const { return Inventory; }

	UFUNCTION(BlueprintPure, Category = "Getters")
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ItemMesh)
	TObjectPtr<class USkeletalMesh> ItemMesh;

	UFUNCTION()
	virtual void OnRep_ItemMesh();
public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetItemMesh(class USkeletalMesh* NewMesh)
	{
		ItemMesh = NewMesh;
		OnRep_ItemMesh();
	}
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Objects/InteractInterface.h"
#include "ItemPickup.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AItemPickup : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	AItemPickup();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Item Pickup")
	void Initialize(class AItem* NewItem);

protected:
	virtual void Interact_Implementation(AShooterCharacter* Interactor) override;
	virtual void Inspect_Implementation(AShooterCharacter* Interactor) override;
	virtual void EndInspect_Implementation(AShooterCharacter* Interactor) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USphereComponent> OverlapSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> ItemMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "State")
	TObjectPtr<class AItem> Item;
	
public:
	
};
 
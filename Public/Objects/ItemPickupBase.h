// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Objects/InteractInterface.h"
#include "ItemPickupBase.generated.h"

/* Item pickup base class.
 * Subclass to add implementations for
 * inspecting and interacting with the
 * item pickup. Call static function for
 * spawning and initialization.
 **/
UCLASS(Abstract)
class MULTIPLAYERSHOOTER_API AItemPickupBase : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	AItemPickupBase();

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void SphereHit(class UPrimitiveComponent* HitComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USphereComponent> OverlapSphere;
	
	UPROPERTY(VisibleInstanceOnly, Category = "State")
	float BobProgress = 0.f;

	// The point to spin about
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USceneComponent> MeshRoot;
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
	bool bDoItemAnim = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bDoItemAnim"), Category = "Configurations")
	float SpinRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bDoItemAnim"), Category = "Configurations")
	float BobRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bDoItemAnim"), Category = "Configurations")
	float BobAmount = 0.1f;
};
 
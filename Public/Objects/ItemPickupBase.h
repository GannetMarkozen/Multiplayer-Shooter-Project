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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void SphereHit(class UPrimitiveComponent* HitComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USphereComponent> OverlapSphere;
	
	UPROPERTY(VisibleInstanceOnly, Category = "State")
	float BobProgress = 0.f;

	// Initialize this when spawning
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> Mesh;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_SkeletalMesh, Category = "Item Pickup")
	class USkeletalMesh* SkeletalMesh;

	UFUNCTION()
	FORCEINLINE void OnRep_SkeletalMesh() { Mesh->SetSkeletalMesh(SkeletalMesh); }
	
public:
	UFUNCTION(BlueprintCallable, Category = "Item Pickup")
	FORCEINLINE void SetSkeletalMesh(class USkeletalMesh* NewMesh)
	{
		SkeletalMesh = NewMesh;
		OnRep_SkeletalMesh();
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
	bool bDoItemAnim = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bDoItemAnim"), Category = "Configurations")
	float SpinRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bDoItemAnim"), Category = "Configurations")
	float BobRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bDoItemAnim"), Category = "Configurations")
	float BobAmount = 0.1f;
};
 
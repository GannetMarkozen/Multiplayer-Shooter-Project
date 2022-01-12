// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ItemPickupBase.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "Kismet/KismetMathLibrary.h"

#include "MultiplayerShooter/MultiplayerShooter.h"
#include "Net/UnrealNetwork.h"


AItemPickupBase::AItemPickupBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Overlap Sphere"));
	OverlapSphere->SetSimulatePhysics(true);
	OverlapSphere->SetNotifyRigidBodyCollision(true);
	OverlapSphere->BodyInstance.bLockRotation = true;
	OverlapSphere->SetCollisionObjectType(ECC_ItemDrop);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OverlapSphere->SetCollisionResponseToChannel(ECC_ItemDrop, ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Projectile, ECR_Block);
	OverlapSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	OverlapSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = OverlapSphere;

	MeshRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Mesh Root"));
	MeshRoot->SetupAttachment(RootComponent);

	SpinRate = 15.f;
	BobRate = 2.5f;
	BobAmount = 0.1f;
}

void AItemPickupBase::BeginPlay()
{
	Super::BeginPlay();

	OverlapSphere->OnComponentHit.AddDynamic(this, &AItemPickupBase::SphereHit);
}


void AItemPickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bDoItemAnim)
	{
		static constexpr float Loop = 2 * PI;

		// Bobbing calculations
		BobProgress += DeltaTime * BobRate;
		if(BobProgress >= Loop) BobProgress -= Loop;
		MeshRoot->AddRelativeLocation({0.f, 0.f, FMath::Sin(BobProgress) * BobAmount});
		MeshRoot->AddLocalRotation({0.f, DeltaTime * SpinRate, 0.f});
	}
}

void AItemPickupBase::SphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic && NormalImpulse.Z >= 0.85f)
	{
		OverlapSphere->SetWorldRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
		OverlapSphere->SetSimulatePhysics(false);
		OverlapSphere->OnComponentHit.RemoveAll(this);
	}
}



// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Weapons/Item.h"

#include "MultiplayerShooter/MultiplayerShooter.h"


AItemPickup::AItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Overlap Sphere"));
	RootComponent = OverlapSphere;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Item Mesh"));
	ItemMesh->SetupAttachment(RootComponent);
}

void AItemPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItemPickup::Initialize_Implementation(AItem* NewItem)
{
	Item = NewItem;
	//ItemMesh->SetSkeletalMesh(NewItem->GetTP_Mesh()->SkeletalMesh);
}


void AItemPickup::Interact_Implementation(AShooterCharacter* Interactor)
{
	
}

void AItemPickup::Inspect_Implementation(AShooterCharacter* Interactor)
{
	
}

void AItemPickup::EndInspect_Implementation(AShooterCharacter* Interactor)
{
	
}



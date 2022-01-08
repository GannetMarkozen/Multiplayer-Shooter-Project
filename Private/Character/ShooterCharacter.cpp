// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterCharacter.h"

#include "AbilitySystemGlobals.h"
#include "GameplayEffectExtension.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/GASAttributeSet.h"
#include "GAS/GASGameplayAbility.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "GAS/Effects/DeathEffect.h"
#include "Net/UnrealNetwork.h"
#include "GAS/GASBlueprintFunctionLibrary.h"


AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetOwnerNoSee(true);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;
	Camera->SetupAttachment(RootComponent);
	
	FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FP_Mesh->SetOnlyOwnerSee(true);
	FP_Mesh->SetupAttachment(Camera);

	ASC = CreateDefaultSubobject<UGASAbilitySystemComponent>(TEXT("Ability System Component"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	Attributes = CreateDefaultSubobject<UGASAttributeSet>(TEXT("Attributes"));

	Inventory = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("Inventory Component"));

	DeathEffectClass = UDeathEffect::StaticClass();
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
		ASC->GetGameplayAttributeValueChangeDelegate(UGASAttributeSet::GetHealthAttribute()).AddUObject(this, &AShooterCharacter::HealthChanged);
}


void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*
	if(GetCharacterMovement()->Velocity.Size() < StationaryThreshold)
	{// If stationary
		if(!bIsLerpRotating)
		{
			const float AddYaw = GetCharacterMovement()->GetLastUpdateRotation().Yaw;
			if(AddYaw != 0.f)
			{
				GetMesh()->AddLocalRotation(FRotator(0.f, -AddYaw, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
				if(GetMesh()->GetRelativeRotation().Yaw + 90.f >= StationaryYawThreshold)
					bIsLerpRotating = true;
			}
		}
		else
		{
			const float AddYaw = FMath::FInterpConstantTo(GetMesh()->GetRelativeRotation().Yaw, -90.f, DeltaTime, InterpYawSpeed);
			GetMesh()->AddLocalRotation(FRotator(0.f, AddYaw, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
			if(FMath::IsNearlyZero(AddYaw)) bIsLerpRotating = false;
		}
	}
	else bIsLerpRotating = false;*/
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(AShooterCharacter, CurrentWeapon, COND_None, REPNOTIFY_OnChanged);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	
	if(ASC && InputComponent)
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EAbilityInput",
		static_cast<int32>(EAbilityInput::Confirm), static_cast<int32>(EAbilityInput::Cancel));
		ASC->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	ASC->InitAbilityActorInfo(this, this);

	InitializeAttributes();

	// Only server grants abilities
	InitializeAbilities();
}

void AShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ASC->InitAbilityActorInfo(this, this);
	
	InitializeAttributes();

	// Only bind input locally
	if(ASC && InputComponent)
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EAbilityInput",
		static_cast<int32>(EAbilityInput::Confirm), static_cast<int32>(EAbilityInput::Cancel));
		ASC->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}

void AShooterCharacter::InitializeAbilities()
{
	if(!HasAuthority()) return;
	for(int32 i = 0; i < DefaultAbilities.Num(); i++)
	{
		if(IsValid(DefaultAbilities[i]))
		{
			const FGameplayAbilitySpec AbilitySpec(DefaultAbilities[i], 1, static_cast<int32>(DefaultAbilities[i].GetDefaultObject()->Input), this);
			const FGameplayAbilitySpecHandle& Handle = ASC->GiveAbility(AbilitySpec);
		}
	}

	PostInitializeAbilities();
}


void AShooterCharacter::InitializeAttributes()
{
	for(const TSubclassOf<class UGameplayEffect>& DefaultEffect : DefaultEffects)
	{
		if(DefaultEffect)
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DefaultEffect, 1, EffectContext);
			if(SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void AShooterCharacter::OnRep_Weapon_Implementation(const AWeapon* LastWeapon)
{
	if(LastWeapon)
	{
		LastWeapon->GetFP_Mesh()->SetVisibility(false);
		LastWeapon->GetTP_Mesh()->SetVisibility(false);
	}
	if(CurrentWeapon)
	{
		CurrentWeapon->GetFP_Mesh()->SetVisibility(true);
		CurrentWeapon->GetTP_Mesh()->SetVisibility(true);

		// Play third person equip montage to all instances
		if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			if(UAnimMontage* Montage = CurrentWeapon->GetTP_EquipMontage())
				AnimInstance->Montage_Play(Montage);

		// If locally controlled, play first person equip montage
		if(IsLocallyControlled())
			if(UAnimInstance* AnimInstance = GetFP_Mesh()->GetAnimInstance())
				if(UAnimMontage* Montage = CurrentWeapon->GetFP_EquipMontage())
					AnimInstance->Montage_Play(Montage);
	}
	
	ChangedWeaponsDelegate.Broadcast(CurrentWeapon, LastWeapon);
}


void AShooterCharacter::Die(const FGameplayEffectSpecHandle& OptionalSpec)
{
	if(!HasAuthority())
	{
		Server_Die(OptionalSpec.IsValid() ? *OptionalSpec.Data.Get() : FGameplayEffectSpec());
	}
	else
	{
		Server_Death(0.f, OptionalSpec);
	}
}

void AShooterCharacter::HealthChanged(const FOnAttributeChangeData& Data)
{
	if(Data.NewValue <= 0.f && Data.GEModData)
	{
		Server_Death(Data.OldValue - Data.NewValue, FGameplayEffectSpecHandle(new FGameplayEffectSpec(Data.GEModData->EffectSpec)));
	}
}

void AShooterCharacter::Server_Death_Implementation(const float Magnitude, const FGameplayEffectSpecHandle& Spec)
{// Set is dead to true to replicate death to clients and call death func server-side as well
	Death(Magnitude, Spec);
	Multi_Death(Magnitude, Spec.IsValid() ? *Spec.Data.Get() : FGameplayEffectSpec());
}

void AShooterCharacter::Death_Implementation(const float Magnitude, const FGameplayEffectSpecHandle& Spec)
{// Called on all instances
	if(DeathEffectClass && !ASC->HasMatchingGameplayTag(TAG("Status.State.Dead")))
	{
		const FGameplayEffectSpecHandle& DeathSpec = ASC->MakeOutgoingSpec(DeathEffectClass, 1.f, ASC->MakeEffectContextExtended(this));
		if(DeathSpec.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*DeathSpec.Data.Get());
			PRINT(TEXT("%s: Applied death effect"), *AUTHTOSTRING(HasAuthority()));
		}
	}
	
	// Start ragdolling
	Ragdoll(Magnitude, Spec);
}

void AShooterCharacter::Ragdoll_Implementation(const float Magnitude, const FGameplayEffectSpecHandle& Spec)
{
	// Ragdoll third person mesh
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllPhysicsLinearVelocity(GetCharacterMovement()->Velocity);

	// Disable capsule collision and input
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// Drop current weapon
	if(CurrentWeapon)
	{
		CurrentWeapon->GetTP_Mesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		CurrentWeapon->GetTP_Mesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CurrentWeapon->GetTP_Mesh()->SetSimulatePhysics(true);
		CurrentWeapon->GetTP_Mesh()->SetAllPhysicsLinearVelocity(GetCharacterMovement()->Velocity);
	}

	// If damage spec is valid, apply velocities associated with damage
	if(Spec.IsValid() && Spec.Data.Get()->GetEffectContext().IsValid())
	{
		FGameplayCueParameters Params;
		Params.RawMagnitude = Magnitude;
		Params.EffectContext = FGameplayEffectContextHandle(new FGameplayEffectContextExtended(this, GAS::FilterTargetDataByActor(this, Extend(Spec.Data.Get()->GetEffectContext().Get())->GetTargetData())));
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(this, TAG("GameplayCue.Knockback"), EGameplayCueEvent::Executed, Params);
	}
}














void AShooterCharacter::MoveForward(float Value)
{
	if(Value != 0.f) AddMovementInput(GetActorForwardVector() * Value);
}

void AShooterCharacter::MoveRight(float Value)
{
	if(Value != 0.f) AddMovementInput(GetActorRightVector() * Value);
}

void AShooterCharacter::LookUp(float Value)
{
	if(Value != 0.f) AddControllerPitchInput(-Value * Sensitivity);
}

void AShooterCharacter::Turn(float Value)
{
	if(Value != 0.f) AddControllerYawInput(Value * Sensitivity);
}




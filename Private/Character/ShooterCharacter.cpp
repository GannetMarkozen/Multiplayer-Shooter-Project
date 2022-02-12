// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterCharacter.h"

#include "AbilitySystemGlobals.h"
#include "GameplayEffectExtension.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/AttributeSets/AmmoAttributeSet.h"
#include "GAS/GASGameplayAbility.h"
#include "GAS/Abilities/Weapons/Weapon.h"
#include "GAS/Effects/DeathEffect.h"
#include "Net/UnrealNetwork.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/Abilities/Weapons/RecoilInstance.h"


AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->bCastHiddenShadow = true;
	GetMesh()->bVisibleInReflectionCaptures = true;
	GetMesh()->SetTickGroup(ETickingGroup::TG_PostUpdateWork); // Set this tick group to late to get camera transform during anim

	ClientOnlyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Client Only Mesh"));
	ClientOnlyMesh->SetCastShadow(false);
	ClientOnlyMesh->bCastHiddenShadow = false;
	ClientOnlyMesh->bVisibleInReflectionCaptures = false;
	ClientOnlyMesh->SetTickGroup(ETickingGroup::TG_PostUpdateWork);
	ClientOnlyMesh->SetupAttachment(GetMesh());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;
	Camera->SetupAttachment(GetMesh(), FName("head"));

	ASC = CreateDefaultSubobject<UGASAbilitySystemComponent>(TEXT("Ability System Component"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	CharacterSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("Character Attribute Set"));
	AmmoSet = CreateDefaultSubobject<UAmmoAttributeSet>(TEXT("Ammo Attribute Set"));

	Inventory = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("Inventory Component"));

	DeathEffectClass = UDeathEffect::StaticClass();
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Init aiming timeline
	if(AimingCurve)
	{
		FOnTimelineFloatStatic TimelineFloat;
		TimelineFloat.BindUObject(this, &AShooterCharacter::AimingTimelineProgress);
		AimingTimeline.AddInterpFloat(AimingCurve, TimelineFloat);

		FOnTimelineEventStatic TimelineEvent;
		TimelineEvent.BindUObject(this, &AShooterCharacter::AimingTimelineEvent);
		AimingTimeline.SetTimelineFinishedFunc(TimelineEvent);
	}

	// Init client mesh
	if(IsLocallyControlled())
	{
		ClientOnlyMesh->HideBoneByName(FName("neck_01"), EPhysBodyOp::PBO_None);
		GetMesh()->SetVisibility(false);
	}
	else
	{
		ClientOnlyMesh->DestroyComponent();
	}

	// Reset recoil instances
	URecoilInstance::NumInstances = 0;

	// Do not predict health
	if(HasAuthority())
		ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetHealthAttribute()).AddUObject(this, &AShooterCharacter::HealthChanged);

	// Predict movement speed change
	ASC->GetGameplayAttributeValueChangeDelegate(UCharacterAttributeSet::GetMovementSpeedAttribute()).AddUObject(this, &AShooterCharacter::MovementSpeedChangedData);

	const float MovementSpeedValue = CharacterSet->MovementSpeed.GetCurrentValue();
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedValue;
	GetCharacterMovement()->MaxWalkSpeedCrouched = MovementSpeedValue * 0.5f;
}


void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimingTimeline.TickTimeline(DeltaTime);
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AShooterCharacter, ADSValue, COND_SkipOwner);
}

void AShooterCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	//DOREPLIFETIME_ACTIVE_OVERRIDE(AShooterCharacter, ADSValue, ADSValue <= 0.f || ADSValue >= 1.f);
}

void AShooterCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
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
			/*const FGameplayAbilitySpecHandle& Handle = */ASC->GiveAbility(AbilitySpec);
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

void AShooterCharacter::MovementSpeedChangedData(const FOnAttributeChangeData& Data)
{
	// Bad implementation, dunno why NewValue is not set to the actual new value.
	const float NewValue = CharacterSet->MovementSpeed.GetCurrentValue();
	const float OldValue = Data.OldValue;
	
	GetCharacterMovement()->MaxWalkSpeed = NewValue;
	GetCharacterMovement()->MaxWalkSpeedCrouched = NewValue * 0.5f;
	BP_MovementSpeedChanged(NewValue, OldValue);
}


void AShooterCharacter::HealthChanged(const FOnAttributeChangeData& Data)
{
	if(Data.NewValue <= 0.f && Data.GEModData && !ASC->HasMatchingGameplayTag(TAG("Status.State.Dead")))
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
	ASC->AddLooseGameplayTag(TAG("Status.State.Dead"));
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
	/*
	if(CurrentWeapon)
	{
		CurrentWeapon->GetTP_Mesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		CurrentWeapon->GetTP_Mesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CurrentWeapon->GetTP_Mesh()->SetSimulatePhysics(true);
		CurrentWeapon->GetTP_Mesh()->SetAllPhysicsLinearVelocity(GetCharacterMovement()->Velocity);
	}*/

	// If damage spec is valid, apply velocities associated with damage
	if(Spec.IsValid() && Spec.Data.Get()->GetEffectContext().IsValid())
	{
		FGameplayCueParameters Params;
		Params.RawMagnitude = Magnitude;
		Params.EffectContext = FGameplayEffectContextHandle(new FGameplayEffectContextExtended(this, GAS::FilterTargetDataByActor(this, Extend(Spec.Data.Get()->GetEffectContext().Get())->GetTargetData())));
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(this, TAG("GameplayCue.Knockback"), EGameplayCueEvent::Executed, Params);
	}
}

/*
 *	AIMING
 */

void AShooterCharacter::StartAiming(const float PlaySpeed)
{
	if(HasAuthority() || IsLocallyControlled())
		Multi_StartAiming_Implementation(PlaySpeed);

	if(!HasAuthority())
		Server_StartAiming(PlaySpeed);
}

void AShooterCharacter::Multi_StartAiming_Implementation(const float PlaySpeed)
{
	AimingTimeline.SetPlayRate(PlaySpeed);
	AimingTimeline.Play();
}


void AShooterCharacter::ReverseAiming()
{
	if(HasAuthority() || IsLocallyControlled())
		Multi_ReverseAiming_Implementation();

	if(!HasAuthority())
		Server_ReverseAiming();
}

void AShooterCharacter::Multi_ReverseAiming_Implementation()
{
	AimingTimeline.Reverse();
}

void AShooterCharacter::AimingComplete_Implementation(const bool bAiming)
{

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




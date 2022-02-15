// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Weapons/Weapon.h"

#include "MultiplayerShooter/Public/GAS/Abilities/Weapons/InputBinding.h"
#include "Character/ShooterCharacter.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "GAS/GASGameplayAbility.h"
#include "GAS/Abilities/Weapons/RecoilInstance.h"
#include "GAS/Abilities/Weapons/Rifle.h"
#include "GAS/AttributeSets/AmmoAttributeSet.h"
#include "GAS/Effects/DamageEffect.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	DamageEffect = UDamageEffect::StaticClass();
	WeaponAttachmentSocketName = FName("weaponsocket_r");

	DefaultScene = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene"));
	RootComponent = DefaultScene;
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionObjectType(ECC_Pawn);
	Mesh->CanCharacterStepUpOn = ECB_No;
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	Mesh->SetCollisionResponseToChannel(ECC_ItemDrop, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Projectile, ECR_Ignore);
	Mesh->SetupAttachment(DefaultScene);
	
	
}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::SetupInputBindings()
{
	//SetupBind(&AWeapon::PrintSomething, EInputBinding::PrimaryFire, IE_Released);
	for(const FInputBindingInfo& Bind : Binds)
		UInputBinding::BindInputUFunction(CurrentOwner, this, Bind.FuncName, Bind.InputBind, Bind.InputEvent);
}

void AWeapon::RemoveInputBindings()
{// Should fix this function being called twice when dropping an item and then immediately swapping to next item, but doesn't really matter
	UInputBinding::RemoveAllInputUObject(CurrentOwner, this);
}



void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(AWeapon, CurrentInventory, COND_None, REPNOTIFY_OnChanged);
}

void AWeapon::OnObtained(UInventoryComponent* Inventory)
{
	if(Inventory != CurrentInventory)
	{
		const UInventoryComponent* OldInventory = CurrentInventory;
		CurrentInventory = Inventory;
		OnRep_CurrentInventory(OldInventory);
		
		BP_OnObtained(Inventory);
	}
}

void AWeapon::OnRemoved(UInventoryComponent* Inventory)
{
	RemoveAbilities();
	
	CurrentInventory = nullptr;
	OnRep_CurrentInventory(Inventory);

	BP_OnRemoved(Inventory);
}

void AWeapon::OnEquipped(UCharacterInventoryComponent* Inventory)
{
	SetVisibility(true);

	// Only runs on server
	GiveAbilities();

	// Sometimes current owner isn't initialized yet at this point for some reason
	if(!CurrentOwner)
		CurrentOwner = CastChecked<AShooterCharacter>(Inventory->GetOwner());
	
	if(EquipMontage)
		if(UAnimInstance* AnimInstance = CurrentOwner->GetMesh()->GetAnimInstance())
			AnimInstance->Montage_Play(EquipMontage);

	// Bind custom inputs
	SetupInputBindings();

	BP_OnEquipped(Inventory);
}

void AWeapon::OnUnequipped(UCharacterInventoryComponent* Inventory)
{
	SetVisibility(false);

	// Only runs on server
	RemoveAbilities();

	// Remove input binding from this weapon
	if(CurrentOwner && CurrentOwner->IsLocallyControlled())
		RemoveInputBindings();
		//UInputBinding::RemoveAllInputUObject(CurrentOwner, this);

	BP_OnUnEquipped(Inventory);
}


void AWeapon::GiveAbilities()
{
	if(HasAuthority() && CurrentOwner)
		for(const TSubclassOf<UGASGameplayAbility>& Ability : WeaponAbilities)
			if(Ability)
				ActiveAbilities.Add(CurrentASC->GiveAbility(FGameplayAbilitySpec(Ability, 1, (int32)Ability.GetDefaultObject()->Input)));
}

void AWeapon::RemoveAbilities()
{
	if(HasAuthority() && CurrentOwner)
	{
		for(const FGameplayAbilitySpecHandle& Handle : ActiveAbilities)
		{
			//CurrentASC->CancelAbilityHandle(Handle);
			if(UGASGameplayAbility* Ability = CurrentASC->GetAbilityFromHandle(Handle))
				Ability->ExternalEndAbility();
			CurrentASC->SetRemoveAbilityOnEnd(Handle);
		}
	}
	ActiveAbilities.Empty();
}

void AWeapon::OnRep_CurrentInventory_Implementation(const UInventoryComponent* OldInventory)
{
	if(CurrentInventory)
	{
		// If owner is AShooterCharacter, attach and init vars
		CurrentOwner = Cast<AShooterCharacter>(CurrentInventory->GetOwner());
		if(CurrentOwner)
		{
			// Init vars
			CurrentCharacterInventory = CurrentOwner->GetCharacterInventory();
			CurrentASC = CurrentOwner->GetASC();

			// Invisible if not currently equipped
			if(CurrentCharacterInventory->GetCurrentWeapon() != this)
				SetVisibility(false);

			// Attach to weapon socket point
			AttachToWeaponSocket();
		}
	}
	else
	{
		if(IsValid(CurrentOwner) && CurrentOwner->IsLocallyControlled())
			RemoveInputBindings();
			//UInputBinding::RemoveAllInputUObject(CurrentOwner, this);
			
		CurrentOwner = nullptr;
		CurrentCharacterInventory = nullptr;
		CurrentASC = nullptr;
		SetOwner(nullptr);
	}
}

void AWeapon::AttachToWeaponSocket()
{
	if(!CurrentOwner) return;
	
	// Attach weapon mesh to character
	Mesh->AttachToComponent(CurrentOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachmentSocketName);
	
	if(Mesh->SkeletalMesh && CurrentOwner->ItemMeshDataTable)
	{
		if(const FMeshTableRow* MeshRow = CurrentOwner->ItemMeshDataTable->FindRow<FMeshTableRow>(Mesh->SkeletalMesh->GetFName(), "MeshTableRow on AWeapon"))
		{
			const AWeapon* DefObj = (AWeapon*)GetClass()->GetDefaultObject();
			const FTransform RelativeTransform(MeshRow->RelativeRotation, MeshRow->RelativeLocation);
			Mesh->SetRelativeTransform(DefObj->Mesh->GetRelativeTransform() * RelativeTransform);
		}
	}
}

bool AWeapon::IsLocallyControlledOwner() const
{
	return CurrentOwner && CurrentOwner->IsLocallyControlled();
}

/*
 *	INPUT BINDINGS
 */

/*
template <typename EnumType>
void AWeapon::BindInputEvent(UObject* Object, const FName& FunctionName, const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent)
{
	if(!CurrentOwner || !Object) return;
	FInputActionBinding Binding(UEnumHelpers::GetEnumValueName<EnumType>(EnumValue), InputEvent);
	Binding.bConsumeInput = false;
	Binding.ActionDelegate.BindDelegate(Object, FunctionName);
	if(CurrentOwner && CurrentOwner->Controller && CurrentOwner->Controller->InputComponent)
		CurrentOwner->Controller->InputComponent->AddActionBinding(Binding);
}

template<typename EnumType, typename UserClass, typename ReturnType, typename... ParamTypes>
void AWeapon::BindInputEvent(UserClass* Object, ReturnType(UserClass::* FuncPtr)(ParamTypes...), const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent)
{
	if(!CurrentOwner || !Object) return;
	FInputActionBinding Binding(UEnumHelpers::GetEnumValueName<EnumType>(EnumValue), InputEvent);
	Binding.bConsumeInput = false;
	Binding.ActionDelegate.GetDelegateForManualSet().BindUObject(Object, FuncPtr);
	if(CurrentOwner && CurrentOwner->Controller && CurrentOwner->Controller->InputComponent)
		CurrentOwner->Controller->InputComponent->AddActionBinding(Binding);
}

template<typename EnumType, typename ReturnType, typename... ParamTypes>
void AWeapon::BindInputEvent(const TDelegate<ReturnType(ParamTypes...)>& FuncPtr, const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent)
{
	if(!CurrentOwner) return;
	FInputActionBinding Binding(UEnumHelpers::GetEnumValueName<EnumType>(EnumValue), InputEvent);
	Binding.bConsumeInput = false;
	Binding.ActionDelegate.GetDelegateForManualSet().BindLambda(FuncPtr);
	if(CurrentOwner && CurrentOwner->Controller && CurrentOwner->Controller->InputComponent)
		CurrentOwner->Controller->InputComponent->AddActionBinding(Binding);
}


template<typename EnumType>
void AWeapon::RemoveInputEventFromObject(UObject* Object, const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent)
{
	if(!CurrentOwner || !Object || !CurrentOwner->Controller || !CurrentOwner->Controller->InputComponent) return;
	
	TArray<FInputKeyBinding>& KeyBindings = CurrentOwner->Controller->InputComponent->KeyBindings;
	for(int32 i = 0; i < KeyBindings.Num(); i++)
		if(KeyBindings[i].Chord.Key.ToString() == UEnumHelpers::GetEnumValueString<EnumType>(EnumValue) && KeyBindings[i].KeyEvent == InputEvent && KeyBindings[i].KeyDelegate.GetUObject() == Object)
		{
			KeyBindings.RemoveAt(i);
			PRINT(TEXT("Removing input %s"), *KeyBindings[i].Chord.Key.ToString());
		}
			
		
		//CurrentOwner->Controller->InputComponent->RemoveActionBinding(UEnumHelpers::GetEnumValueName<EnumType>(EnumValue), InputEvent);
}
*/
/*
template<typename UserClass, typename ReturnType, typename... ParamTypes, typename EnumType>
void AWeapon::Internal_SetBinding(ReturnType(UserClass::* MemFuncPtr)(ParamTypes...), const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent, const bool bRemoving)
{
	bRemoving ? UInputBinding::RemoveInputUObject(CurrentOwner, this, EnumValue, InputEvent) : UInputBinding::BindInputUObject(CurrentOwner, this, MemFuncPtr, EnumValue, InputEvent);
}*/
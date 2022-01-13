#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GASAbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/Weapons/Weapon.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAS/ExtendedTypes.h"

#include "GASBlueprintFunctionLibrary.generated.h"

#define GAS UGASBlueprintFunctionLibrary

UENUM(BlueprintType)
enum class EDisplayDamage : uint8
{
	None, LocalOnly, NetMulticast
};

UCLASS()
class MULTIPLAYERSHOOTER_API UGASBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Returns the target data handle from the given effect context handle
	UFUNCTION(BlueprintPure, Meta = (DisplayName = "Get Target Data"), Category = "GAS")
	static FORCEINLINE FGameplayAbilityTargetDataHandle GetTargetDataHandle(const FGameplayEffectContextHandle& EffectContextHandle)
	{
		if(const FGameplayEffectContextExtended* EffectContext = (const FGameplayEffectContextExtended*)EffectContextHandle.Get())
		{
			return EffectContext->GetTargetData();
		}
		else return FGameplayAbilityTargetDataHandle();
	}

	UFUNCTION(BlueprintCallable, Meta = (DisplayName = "Add Target Data"), Category = "GAS")
	static FORCEINLINE void AddTargetDataHandle(const FGameplayEffectContextHandle& EffectContextHandle, const FGameplayAbilityTargetDataHandle& TargetDataHandle)
	{
		if(FGameplayEffectContextExtended* EffectContext = (FGameplayEffectContextExtended*)EffectContextHandle.Get())
		{
			EffectContext->AddTargetData(TargetDataHandle);
		}
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE FGameplayEffectContextHandle GetEffectContext(const FGameplayEffectSpecHandle& EffectSpec)
	{
		if(EffectSpec.IsValid())
			return EffectSpec.Data.Get()->GetEffectContext();
		else return FGameplayEffectContextHandle();
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static const FORCEINLINE TMap<FGameplayTag, float>& GetSetByCallerMagnitudes(const FGameplayEffectSpecHandle& EffectSpec)
	{
		return EffectSpec.Data.Get()->SetByCallerTagMagnitudes;
	}

	// Returns the number of hit results with the actor to check from the target data handle
	UFUNCTION(BlueprintPure, Meta = (DefaultToSelf = "Target", DisplayName = "Get Number Of Compounding Hits"), Category = "GAS")
	static FORCEINLINE int32 GetNumCompoundingHits(const class AActor* ActorToCheck, const FGameplayAbilityTargetDataHandle& TargetData)
	{
		int32 NumHits = 0;
		for(int32 i = 0; i < TargetData.Data.Num(); i++)
		{
			if(!TargetData.IsValid(i)) continue;
			if(const FHitResult* Hit = TargetData.Data[i].Get()->GetHitResult())
			{
				if(Hit->GetActor() == ActorToCheck)
					NumHits++;
			}
		}
		return NumHits;
	}

	// Derives the number of hits on the ActorToCheck with the BaseDamage of the effect
	UFUNCTION(BlueprintPure, Meta = (DefaultToSelf = "Target"), Category = "GAS")
	static FORCEINLINE float GetCompoundingDamage(class AActor* ActorToCheck, const FGameplayEffectSpecHandle& EffectSpec, const float ValueToReturnOnFailed = 0.f)
	{
		if(EffectSpec.IsValid() && EffectSpec.Data.Get()->GetEffectContext().IsValid())
		{
			if(const float* BaseDamage = EffectSpec.Data.Get()->SetByCallerTagMagnitudes.Find(UAbilitySystemGlobalsExtended::Get().GetBaseDamageTag()))
			{
				return *BaseDamage * static_cast<float>(GetNumCompoundingHits(ActorToCheck, ((FGameplayEffectContextExtended*)EffectSpec.Data.Get()->GetEffectContext().Get())->GetTargetData()));
			}
		}

		return ValueToReturnOnFailed;
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE void GetHitArray(const FGameplayAbilityTargetDataHandle& TargetData, TArray<FHitResult>& OutHits)
	{
		for(int32 i = 0; i < TargetData.Data.Num(); i++)
		{
			if(TargetData.IsValid(i))
			{
				if(const FHitResult* Hit = TargetData.Data[i].Get()->GetHitResult())
				{
					OutHits.Add(*Hit);
				}
			}
		}
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static const FORCEINLINE FGameplayEffectContextHandle& GetContext(const FGameplayCueParameters& Params)
	{
		return Params.EffectContext;
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE float GetRawMagnitude(const FGameplayCueParameters& Params)
	{
		return Params.RawMagnitude;
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE float GetNormalizedMagnitude(const FGameplayCueParameters& Params)
	{
		return Params.NormalizedMagnitude;
	}

	UFUNCTION(BlueprintCallable, Category = "GAS")
	static FORCEINLINE void SetTarget(const FGameplayEffectContextHandle& Context, class AActor* Target)
	{
		((FGameplayEffectContextExtended*)Context.Get())->SetTarget(Target);
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE class AActor* GetTarget(const FGameplayEffectContextHandle& Context)
	{
		return ((FGameplayEffectContextExtended*)Context.Get())->GetTarget();
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE class UGASAbilitySystemComponent* GetInstigatorAbilitySystemComponent(const FGameplayEffectContextHandle& Context)
	{
		return (UGASAbilitySystemComponent*)Context.GetInstigatorAbilitySystemComponent();
	}

	// Creates new handle with filtered target data
	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE FGameplayAbilityTargetDataHandle FilterTargetDataByActor(const class AActor* ActorToFilter, const FGameplayAbilityTargetDataHandle& TargetData)
	{
		FGameplayAbilityTargetDataHandle Handle;
		for(int32 i = 0; i < TargetData.Data.Num(); i++)
		{
			const FGameplayAbilityTargetData* Data = TargetData.Data[i].Get();
			if(Data && Data->GetActors().Contains(ActorToFilter))
			{
				int32 Size = Data->GetScriptStruct()->GetStructureSize();
				FGameplayAbilityTargetData* NewData = (FGameplayAbilityTargetData*)new char[Size];
				char* NewDataItr = (char*)NewData;
				char* DataItr = (char*)Data;
				while(Size--) *NewDataItr++ = *DataItr++;
				Handle.Add(NewData);
			}
		}
		return Handle;
	}

	// Creates new handle with data that is confirmed using the TFunction<bool(const FGameplayAbilityTargetData&)> filter
	static FORCEINLINE FGameplayAbilityTargetDataHandle FilterTargetDataBy(const FGameplayAbilityTargetDataHandle& TargetData, const TFunction<bool(const FGameplayAbilityTargetData&)>& Params)
	{
		FGameplayAbilityTargetDataHandle Handle;
		for(int32 i = 0; i < TargetData.Data.Num(); i++)
		{
			const FGameplayAbilityTargetData* Data = TargetData.Data[i].Get();
			if(Data && Params(*Data))
			{
				int32 Size = TargetData.Data[i].Get()->GetScriptStruct()->GetStructureSize();
				FGameplayAbilityTargetData* NewData = (FGameplayAbilityTargetData*)new char[Size];
				char* NewDataItr = (char*)NewData;
				char* DataItr = (char*)Data;
				while(Size--) *NewDataItr++ = *DataItr++;
				Handle.Add(NewData);
			}
		}
		return Handle;
	}
	
	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE FGameplayAbilityTargetDataHandle MakeTargetDataFromHits(const TArray<FHitResult>& Hits)
	{
		FGameplayAbilityTargetDataHandle Handle;
		for(const FHitResult& Hit : Hits)
			Handle.Add(new FGameplayAbilityTargetData_SingleTargetHit(Hit));
		return Handle;
	}

	// Template of target data struct type you want to extrapolate
	template<typename T>
	static FORCEINLINE FGameplayAbilityTargetDataHandle FilterTargetData(const FGameplayAbilityTargetDataHandle& TargetData)
	{
		FGameplayAbilityTargetDataHandle Handle;
		for(int32 i = 0; i < TargetData.Data.Num(); i++)
		{
			if(TargetData.Data[i].IsValid() && TargetData.Data[i].Get()->GetScriptStruct() == (UScriptStruct*)T::StaticStruct())
			{
				Handle.Add(new T(static_cast<T&>(*TargetData.Data[i].Get())));
			}
		}
		return Handle;
	}

	// Template of target data struct type you want to extrapolate. Puts all data into a TArray<T*>
	template<typename T>
	static FORCEINLINE TArray<T*> FilterTargetDataArray(const FGameplayAbilityTargetDataHandle& TargetData)
	{
		TArray<T*> Data;
		for(int32 i = 0; i < TargetData.Data.Num(); i++)
		{
			if(TargetData.Data[i].IsValid() && TargetData.Data[i].Get()->GetScriptStruct() == T::StaticStruct())
			{
				Data.Add(static_cast<T*>(TargetData.Data[i].Get()));
			}
		}
		return Data;
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE void GetFilteredHitArray(const FGameplayAbilityTargetDataHandle& TargetData, const class AActor* Filter, TArray<FHitResult>& OutHits)
	{
		GetHitArray(TargetData, OutHits);
		for(int32 i = 0; i < OutHits.Num(); i++)
			if(OutHits[i].GetActor() != Filter) OutHits.RemoveAt(i);
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static const FORCEINLINE FGameplayAbilityActorInfoExtended& ExtendActorInfo(const FGameplayAbilityActorInfo& ActorInfo)
	{
		return (const FGameplayAbilityActorInfoExtended&)ActorInfo;
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE class AShooterCharacter* GetCharacter(const FGameplayAbilityActorInfo& ActorInfo)
	{
		return ((const FGameplayAbilityActorInfoExtended&)ActorInfo).Character.Get();
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE class UCharacterInventoryComponent* GetCharacterInventory(const FGameplayAbilityActorInfo& ActorInfo)
	{
		return ((const FGameplayAbilityActorInfoExtended&)ActorInfo).Inventory.Get();
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE class UGASAbilitySystemComponent* GetASC(const FGameplayAbilityActorInfo& ActorInfo)
	{
		return ((const FGameplayAbilityActorInfoExtended&)ActorInfo).ASC.Get();
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE bool IsNetAuthority(const FGameplayAbilityActorInfoExtended& ActorInfo)
	{
		return ActorInfo.IsNetAuthority();
	}

	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE bool IsLocallyControlled(const FGameplayAbilityActorInfoExtended& ActorInfo)
	{
		return ActorInfo.IsLocallyControlled();
	}

	UFUNCTION(BlueprintPure, Meta = (DefaultToSelf = "Instigator", AutoCreateRefTerm = "TargetData"), Category = "GAS")
	static FORCEINLINE FGameplayEffectContextHandle MakeEffectContextHandle(const class UAbilitySystemComponent* ASC, class AActor* Target, const FGameplayAbilityTargetDataHandle& TargetData)
	{
		if(!ASC) return FGameplayEffectContextHandle();
		const FGameplayEffectContextHandle& Context = ASC->MakeEffectContext();
		((FGameplayEffectContextExtended*)Context.Get())->SetTarget(Target);
		((FGameplayEffectContextExtended*)Context.Get())->AddTargetData(TargetData);
		return Context;
	}

	UFUNCTION(BlueprintPure, Meta = (AutoCreateRefTerm = "Name"), Category = "GAS")
	static FORCEINLINE bool FindMeshTableRow(const class UDataTable* DataTable, const FName& Name, FMeshTableRow& MeshTableRow)
	{
		if(!DataTable) return false;
		if(const FMeshTableRow* Query = DataTable->FindRow<FMeshTableRow>(Name, "Context"))
		{
			MeshTableRow = *Query;
			return true;
		}
		return false;
	}

	// Tests whether component is part of collision channel that should have force applied and that it is simulating physics
	UFUNCTION(BlueprintPure, Category = "GAS")
	static FORCEINLINE bool ShouldProjectileApplyForce(const class UPrimitiveComponent* Component, const FName& BoneName = NAME_None)
	{
		if(!Component) return false;
		const ECollisionChannel CC = Component->GetCollisionObjectType();
		return CC != ECC_WorldDynamic && CC != ECC_WorldStatic && CC != ECC_ItemDrop && Component->IsSimulatingPhysics(BoneName);
	}

	// Calculates the damage using the instigator's equipped weapon or base damage set by caller magnitude. Display damage only works if instigator is an AShooterCharacter
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "Target"), Category = "GAS")
	static int32 CalculateDamage(class AActor* Target, class AActor* Instigator, const FGameplayEffectSpecHandle& Spec, const EDisplayDamage DisplayDamage = EDisplayDamage::None);

	// Only works if calculating target data from originating from FGameplayAbilityTargetData_SingleTargetHit or any other that would return a hit result
	UFUNCTION(BlueprintCallable, Category = "GAS")
	static void ApplyDamageKnockback(const FGameplayEffectContextHandle& Context, float Damage, float KnockbackMagnitude = 1.f);
};

UCLASS()
class MULTIPLAYERSHOOTER_API UGASDebugFuncLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Print = true to print on screen or false to log in debug log
	UFUNCTION(BlueprintCallable, Category = "GAS|Debug")
	static FORCEINLINE void PrintAllAbilities(const class UGASAbilitySystemComponent* ASC, const bool bPrint = false)
	{
		FString Message;
		const TArray<FGameplayAbilitySpec>& Specs = ASC->GetActivatableAbilities();
		for(int32 i = 0; i < Specs.Num(); i++)
			if(Specs[i].Ability)
				Message.Append(Specs[i].Ability->GetName() + ", ");
		
		if(bPrint)
		{
			PRINT(TEXT("Abilities == %s"), *Message);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Abilities == %s"), *Message);
		}
	}

	UFUNCTION(BlueprintCallable, Category = "GAS|Debug")
	static FORCEINLINE void PrintDuplicateAbilities(const class UGASAbilitySystemComponent* ASC, const bool bPrint = false)
	{
		FString Message;
		const TArray<FGameplayAbilitySpec>& Specs = ASC->GetActivatableAbilities();
		TArray<UGameplayAbility*> Abilities;
		for(const FGameplayAbilitySpec& Spec : Specs)
		{
			for(const UGameplayAbility* Ability : Abilities)
				if(Ability->GetClass() == Spec.Ability->GetClass())
					PRINT(TEXT("Dupe of ability %s"), *Spec.Ability->GetName());
			
			Abilities.Add(Spec.Ability);
		}
	}
};
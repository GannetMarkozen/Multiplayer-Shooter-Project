#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GAS/ExtendedTypes.h"

#include "GASBlueprintFunctionLibrary.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API UGASBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Returns the target data handle from the given effect context handle
	UFUNCTION(BlueprintPure, Meta = (DisplayName = "GetTargetData"), Category = "GAS")
	static FORCEINLINE FGameplayAbilityTargetDataHandle EffectContextGetTargetDataHandle(const FGameplayEffectContextHandle& EffectContextHandle)
	{
		if(const FGameplayEffectContextExtended* EffectContext = (const FGameplayEffectContextExtended*)EffectContextHandle.Get())
		{
			return EffectContext->GetTargetData();
		}
		else return FGameplayAbilityTargetDataHandle();
	}

	UFUNCTION(BlueprintCallable, Meta = (DisplayName = "AddTargetData"), Category = "GAS")
	static FORCEINLINE void EffectContextAddTargetDataHandle(const FGameplayEffectContextHandle& EffectContextHandle, const FGameplayAbilityTargetDataHandle& TargetDataHandle)
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
	UFUNCTION(BlueprintPure, Meta = (DefaultToSelf = "Target", DisplayName = "GetNumberOfCompoundingHits"), Category = "GAS")
	static FORCEINLINE int32 GetNumCompoundingHits(class AActor* ActorToCheck, const FGameplayAbilityTargetDataHandle& TargetData)
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
	static FORCEINLINE void GetHitsFromTargetData(const FGameplayAbilityTargetDataHandle& TargetData, TArray<FHitResult>& OutHits)
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
};
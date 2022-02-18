#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilities/Public/GameplayEffect.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "GameplayAbilities/Public/GameplayCueManager.h"
#include "Kismet/GameplayStatics.h"

#include "ExtendedTypes.generated.h"


USTRUCT(BlueprintType)
struct FGameplayAbilityActorInfoExtended : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "ActorInfo")
	TWeakObjectPtr<class AShooterCharacter> Character;

	UPROPERTY(BlueprintReadWrite, Category = "ActorInfo")
	TWeakObjectPtr<class UCharacterInventoryComponent> Inventory;

	UPROPERTY(BlueprintReadWrite, Category = "ActorInfo")
	TWeakObjectPtr<class UGASAbilitySystemComponent> ASC;

	virtual void InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;

	virtual void ClearActorInfo() override
	{
		Super::ClearActorInfo();
		Character = nullptr;
		Inventory = nullptr;
	}
};



USTRUCT()
struct FGameplayEffectContextExtended : public FGameplayEffectContext
{
	GENERATED_USTRUCT_BODY()
public:
	FGameplayEffectContextExtended(){}
	FGameplayEffectContextExtended(class AActor* Instigator, const FGameplayAbilityTargetDataHandle& TargetData)
	{
		this->Instigator = Instigator;
		this->TargetData = TargetData;
	}
	
	static FGameplayEffectContextExtended* Get(const FGameplayEffectContext* Other)
	{
		return (FGameplayEffectContextExtended*)Other;
	}
	
	virtual const FGameplayAbilityTargetDataHandle& GetTargetData() const
	{
		return TargetData;
	}
	
	virtual void AddTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
	{
		TargetData.Append(TargetDataHandle);
	}

	// overrides

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayEffectContextExtended::StaticStruct();
	}

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FGameplayEffectContextExtended* NewContext = new FGameplayEffectContextExtended();
		*NewContext = *this;
		NewContext->AddActors(Actors);
		if(GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		NewContext->TargetData.Append(TargetData);
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		const bool bReturn = Super::NetSerialize(Ar, Map, bOutSuccess) && TargetData.NetSerialize(Ar, Map, bOutSuccess);
		CustomTags.NetSerialize(Ar, Map, bOutSuccess);
		
		uint8 RepBits = 0;
		if(Ar.IsSaving())
			RepBits |= 1 << 0;

		Ar.SerializeBits(&RepBits, 1);

		if(RepBits & 1 << 0)
			Ar << Target;

		return bReturn;
	}

	class AActor* GetTarget() const
	{
		return Target.Get();
	}

	FORCEINLINE void SetTarget(class AActor* InTarget)
	{
		Target = InTarget;
	}
	
	FGameplayTagContainer CustomTags;
protected:
	FGameplayAbilityTargetDataHandle TargetData;
	TWeakObjectPtr<class AActor> Target;
};

template<>
struct TStructOpsTypeTraits<FGameplayEffectContextExtended> : public TStructOpsTypeTraitsBase2<FGameplayEffectContextExtended>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

static FORCEINLINE FGameplayEffectContextExtended* Extend(const FGameplayEffectContextHandle& Handle)
{
	return (FGameplayEffectContextExtended*)Handle.Get();
}

static FORCEINLINE FGameplayEffectContextExtended* Extend(const FGameplayEffectContext* Context)
{
	return (FGameplayEffectContextExtended*)Context;
}

static FORCEINLINE const FGameplayAbilityActorInfoExtended* Extend(const FGameplayAbilityActorInfo* ActorInfo)
{
	return (const FGameplayAbilityActorInfoExtended*)ActorInfo;
}

UCLASS()
class MULTIPLAYERSHOOTER_API UAbilitySystemGlobalsExtended : public UAbilitySystemGlobals
{
	GENERATED_BODY()
public:
	UAbilitySystemGlobalsExtended();
	
	static UAbilitySystemGlobalsExtended& Get();
	class UGameplayCueManagerExtended* GetGameplayCueManagerExtended();

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override
	{
		return new FGameplayEffectContextExtended();
	}

	virtual FGameplayAbilityActorInfo* AllocAbilityActorInfo() const override
	{
		return new FGameplayAbilityActorInfoExtended();
	}

	virtual void InitGlobalTags() override
	{
		Super::InitGlobalTags();

		DeadTag = FGameplayTag::RequestGameplayTag(FName("Status.State.Dead"));
		StunnedTag = FGameplayTag::RequestGameplayTag(FName("Status.Debuff.Stunned"));
		BaseDamageTag = FGameplayTag::RequestGameplayTag(FName("Data.BaseDamage"));
	}

	virtual FORCEINLINE void InitGameplayCueParameters_GESpec(FGameplayCueParameters& CueParameters, const FGameplayEffectSpec& Spec) override
	{
		Super::InitGameplayCueParameters_GESpec(CueParameters, Spec);
	}
	
protected:
	UPROPERTY()
	FGameplayTag DeadTag;

	UPROPERTY()
	FGameplayTag StunnedTag;

	UPROPERTY()
	FGameplayTag BaseDamageTag;
	
public:
	const FORCEINLINE FGameplayTag& GetDeadTag() const { return DeadTag; }
	const FORCEINLINE FGameplayTag& GetStunnedTag() const { return StunnedTag; }
	const FORCEINLINE FGameplayTag& GetBaseDamageTag() const { return BaseDamageTag; }
};

UCLASS()
class MULTIPLAYERSHOOTER_API UGameplayCueManagerExtended : public UGameplayCueManager
{
	GENERATED_BODY()
public:
	UGameplayCueManagerExtended(){}
protected:
	virtual FORCEINLINE bool ShouldAsyncLoadRuntimeObjectLibraries() const override { return true; }
	virtual void InvokeGameplayCueExecuted_FromSpec(UAbilitySystemComponent* OwningComponent, const FGameplayEffectSpec& Spec, FPredictionKey PredictionKey) override;
	class UClass* GetGameplayCueNotifyClass(const FGameplayTag& GameplayCueTag);
};




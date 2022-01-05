// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "GASGameplayAbility.h"
#include "GameplayAbilities/Public/AbilitySystemComponent.h"
#include "MultiplayerShooter/MultiplayerShooter.h"

#include "GASAbilitySystemComponent.generated.h"

//DECLARE_DELEGATE_ThreeParams(FAttributeChangedDelegate, float, float, const FGameplayEffectSpecHandle&);

// Object attribute pair hash key for TMap
USTRUCT()
struct FObjAttrPair
{
	GENERATED_BODY()
	UPROPERTY()
	UObject* Obj = nullptr;
	FGameplayAttribute Attr;
	FObjAttrPair(){}
	FObjAttrPair(class UObject* Obj, const FGameplayAttribute& Attr) { this->Obj = Obj; this->Attr = Attr; }
	bool operator==(const FObjAttrPair& Other) const
	{
		return Obj == Other.Obj && Attr == Other.Attr;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FObjAttrPair& Other)
	{
		return FCrc::MemCrc32(&Other, sizeof(FObjAttrPair));
	}
};
// Attribute Changed Parameters
struct AttrChParams
{
	float NewValue;
	float OldValue;
	FGameplayEffectSpecHandle Spec;
	AttrChParams(){}
	AttrChParams(const float NewValue, const float OldValue, const FGameplayEffectSpecHandle& Spec)
	{
		this->NewValue = NewValue; this->OldValue = OldValue; this->Spec = Spec;
	}
};
/*
struct FDelegateHandlePair
{
	FAttributeChangedDelegate AttributeDelegate;
	FDelegateHandle DelegateHandle;
	FDelegateHandlePair(){}
	FDelegateHandlePair(const FAttributeChangedDelegate& AttributeDelegate, const FDelegateHandle& DelegateHandle)
	{
		this->AttributeDelegate = AttributeDelegate; this->DelegateHandle = DelegateHandle;
	}
};*/

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UGASAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "GAS")
	bool BatchRPCTryActivateAbility(const FGameplayAbilitySpecHandle& AbilityHandle, const bool bEndAbilityImmediately);

	UFUNCTION(BlueprintPure, Meta = (AutoCreateRefTerm = "Class"), Category = "GAS")
	TArray<FGameplayAbilitySpecHandle> GetAbilitiesByClass(const TSubclassOf<class UGASGameplayAbility>& Class) const;

	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"), Category = "GameplayCue")
	FORCEINLINE void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters, const bool RPCLocalIfNecessary = true);

	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"), Category = "GameplayCue")
	FORCEINLINE void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"), Category = "GameplayCue")
	FORCEINLINE void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"), Category = "GameplayCue")
	FORCEINLINE void ExecuteGameplayCueNetMulticast(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
	{
		NetMulticast_InvokeGameplayCueExecuted_WithParams(GameplayCueTag, FPredictionKey(), GameplayCueParameters);
	}
 
	UFUNCTION(BlueprintCallable, Meta = (DisplayName = "Make Effect Context Extended"), Category = "GAS")
	FORCEINLINE FGameplayEffectContextHandle K2_MakeEffectContextExtended(class AActor* OptionalTargetActor, const FGameplayAbilityTargetDataHandle OptionalTargetData) const
	{
		return MakeEffectContextExtended(OptionalTargetActor, OptionalTargetData);
	}

	FORCEINLINE FGameplayEffectContextHandle MakeEffectContextExtended(class AActor* OptionalTarget = nullptr, const FGameplayAbilityTargetDataHandle& OptionalTargetDataHandle = FGameplayAbilityTargetDataHandle()) const
	{
		const FGameplayEffectContextHandle& Handle = MakeEffectContext();
		if(!OptionalTargetDataHandle.Data.IsEmpty()) ((FGameplayEffectContextExtended*)Handle.Get())->AddTargetData(OptionalTargetDataHandle);
		if(OptionalTarget) ((FGameplayEffectContextExtended*)Handle.Get())->SetTarget(OptionalTarget);
		return Handle;
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Loose Gameplay Tags"), Category = "GAS")
	FORCEINLINE void BP_AddLooseGameplayTags(const FGameplayTagContainer Tags, const int32 Count = 1) { AddLooseGameplayTags(Tags, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Loose Gameplay Tags"), Category = "GAS")
	FORCEINLINE void BP_RemoveLooseGameplayTags(const FGameplayTagContainer Tags, const int32 Count = 1) { RemoveLooseGameplayTags(Tags, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Add Loose Gameplay Tag"), Category = "GAS")
	FORCEINLINE void BP_AddLooseGameplayTag(const FGameplayTag Tag, const int32 Count = 1) { AddLooseGameplayTag(Tag, Count); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Remove Loose Gameplay Tag"), Category = "GAS")
	FORCEINLINE void BP_RemoveLooseGameplayTag(const FGameplayTag Tag, const int32 Count = 1) { RemoveLooseGameplayTag(Tag, Count); }

	// Removes exact present gameplay tag and children tags
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "Tag"), Category = "GAS")
	FORCEINLINE void RemoveLooseGameplayTagChildren(const FGameplayTag& Tag, const int32 Count = 1)
	{
		if(!Tag.IsValid()) return;
		FGameplayTagContainer Tags;
		GetOwnedGameplayTags(Tags);
		for(TArray<FGameplayTag>::TConstIterator Itr(Tags.CreateConstIterator()); Itr; ++Itr)
			if(Itr->MatchesTag(Tag))
				RemoveLooseGameplayTag(*Itr, Count);
	}

	// Adds loose tag for the given duration
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "Tag"), Category = "GAS")
	void AddLooseGameplayTagForDuration(const FGameplayTag& Tag, const float Duration, const int32 Count = 1);

	// Only adds tag if there is none currently added for a duration. Else sets time for removal to the given duration
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "Tag"), Category = "GAS")
	void AddLooseGameplayTagForDurationSingle(const FGameplayTag& Tag, const float Duration);

	// Binds attribute changing to object function. Params are float (NewValue), float (OldValue), const FGameplayEffectContextHandle& (Context)
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "Object", AutoCreateRefTerm = "Attribute"), Category = "GAS")
	void BindAttributeChanged(class UObject* Object, const FName FuncName, const FGameplayAttribute& Attribute);

	// Unbinds all bindings from this object associated with the attribute
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "Object", AutoCreateRefTerm = "Attribute"), Category = "GAS")
	void UnbindAttributeChanged(class UObject* Object, const FGameplayAttribute& Attribute);
	
protected:
	virtual void AbilityLocalInputPressed(int32 InputID) override;
	virtual void ClientActivateAbilityFailed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, int16 PredictionKey) override;
	virtual void ClientActivateAbilitySucceed_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey) override;
	virtual void ClientActivateAbilitySucceedWithEventData_Implementation(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey PredictionKey, FGameplayEventData TriggerEventData) override;
	virtual FORCEINLINE bool ShouldDoServerAbilityRPCBatch() const override { return true; }
	
	UFUNCTION(Client, Reliable)
	void Client_ExecuteGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& Params);
	FORCEINLINE void Client_ExecuteGameplayCueLocal_Implementation(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& Params)
	{
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Executed, Params);
	}

	// Timer handle TSharedPtrs associated with a gameplay tag
	TMap<FGameplayTag, TSharedPtr<FTimerHandle>> TimerHandles;
	TMap<FObjAttrPair, FDelegateHandle> DelegateHandles;

	UPROPERTY(VisibleInstanceOnly, Category = "TEST")
	int32 NumDelegates = 0;
};

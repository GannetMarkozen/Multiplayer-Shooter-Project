// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "UObject/Object.h"
#include "RecoilInstance.generated.h"

//int32 NumInstances = 0;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRecoilEnd, class URecoilInstance*, RecoilInstance);
/**
 * 
 */
UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, Transient)
class MULTIPLAYERSHOOTER_API URecoilInstance : public UObject
{
	GENERATED_BODY()
public:
	URecoilInstance();

protected:
	virtual bool Tick(const float DeltaTime);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	class UCurveFloat* RecoilCurve;

	UPROPERTY(BlueprintReadOnly, Category = "Configurations")
	float RecoilMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	float RecoilMagnitude = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	float PlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configurations")
	bool bApplyControllerRecoil = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "bApplyControllerRecoil"), Category = "Configurations")
	float ControllerRecoilMultiplier = 0.1f;

	UPROPERTY(BlueprintReadWrite, Category = "State")
	class AShooterCharacter* Wielder;

	UFUNCTION(BlueprintNativeEvent, Category = "Recoil")
	void Activate();
	virtual void Activate_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Recoil")
	FTransform MakeTargetTransform();
	virtual FTransform MakeTargetTransform_Implementation();

	UPROPERTY(BlueprintReadOnly, Category = "Recoil")
	FTransform TargetTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Recoil")
	FTransform CurrentTransform;

	FTickerDelegate TickerDelegate;
	FDelegateHandle TickerDelegateHandle;

	FTimeline RecoilTimeline;

	UFUNCTION(BlueprintNativeEvent, Category = "Recoil")
	void TimelineProgress(const float Value);
	virtual void TimelineProgress_Implementation(const float Value);

	UFUNCTION(BlueprintNativeEvent, Category = "Recoil")
	void TimelineEnd();
	virtual void TimelineEnd_Implementation();

	UFUNCTION()
	virtual FORCEINLINE void WielderDestroyed(AActor* DestroyedActor)
	{
		Destroy();
	}

	virtual FORCEINLINE void Destroy()
	{
		RemoveFromRoot();
		ConditionalBeginDestroy();
		RecoilTimeline.Stop();
	}
	
public:
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "InWielder", AutoCreateRefTerm = "RecoilClass"), Category = "Recoil")
	static class URecoilInstance* AddRecoilInstance(class AShooterCharacter* InWielder, const TSubclassOf<class URecoilInstance>& RecoilClass, const float InRecoilMultiplier = 1.f);

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FRecoilEnd RecoilEnd;

	UFUNCTION(BlueprintPure, Category = "Recoil")
	static FORCEINLINE int32 GetNumInstances() { return NumInstances; }

	static int32 NumInstances;
};
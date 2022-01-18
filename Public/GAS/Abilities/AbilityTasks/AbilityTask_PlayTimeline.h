// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Components/TimelineComponent.h"
#include "GAS/ExtendedTypes.h"
#include "AbilityTask_PlayTimeline.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTimelineProgress, float, DeltaTime, float, Progress, float, Value);

// Will tick in the output pin until task ends
UCLASS()
class MULTIPLAYERSHOOTER_API UAbilityTask_PlayTimeline : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_PlayTimeline();

	UPROPERTY(BlueprintAssignable)
	FTimelineProgress TimelineProgress;

	UPROPERTY(BlueprintAssignable)
	FTimelineProgress TimelineEnd;

	// Ticks the progress. Automatically ends on timeline end
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static class UAbilityTask_PlayTimeline* PlayTimeline(class UGASGameplayAbility* OwningAbility, const FName TaskInstanceName, class UCurveFloat* InCurveFloat, const float InStartTime = 0.f, const float InPlayRate = 1.f,
		const ETimelineDirection::Type InDirection = ETimelineDirection::Forward, const bool bInDestroyOnEnd = true);

	// Reverses timeline from current direction
	UFUNCTION(BlueprintCallable)
	void Reverse();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void PlayForward()
	{
		if(Direction == ETimelineDirection::Backward) Reverse();
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void PlayBackward()
	{
		if(Direction == ETimelineDirection::Forward) Reverse();
	}
	
	virtual void Activate() override;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<ETimelineDirection::Type> Direction;
	
protected:
	virtual void OnDestroy(bool bInOwnerFinished) override;
	virtual void TickTask(float DeltaTime) override;

	UFUNCTION()
	virtual void Progress(float Value);

	UFUNCTION()
	virtual void End();

	UFUNCTION()
	virtual FORCEINLINE void FailedPrediction(const FGameplayAbilityActorInfoExtended& ActorInfo)
	{
		EndTask();
		CurveFTimeline.Stop();
	}

	UFUNCTION()
	virtual FORCEINLINE void AbilityEnded(class UGameplayAbility* EndedAbility)
	{
		EndTask();
		CurveFTimeline.Stop();
	}

	UPROPERTY()
	class UCurveFloat* CurveFloat;

	UPROPERTY()
	FTimeline CurveFTimeline;

	float StartTime;
	
	float PlayRate;
	
	bool bDestroyOnEnd;
	

	float TaskDeltaTime;

	float TimelineStartTime;
	float TimelineEndTime;

	bool bSetupStartTimelineEvent = false;
	bool bSetupEndTimelineEvent = false;
};
// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/AbilityTasks/AbilityTask_PlayTimeline.h"
#include "Abilities/Tasks/AbilityTask_MoveToLocation.h"
#include "Character/ShooterCharacter.h"
#include "GAS/GASGameplayAbility.h"


UAbilityTask_PlayTimeline::UAbilityTask_PlayTimeline()
{
	bTickingTask = true;
}

UAbilityTask_PlayTimeline* UAbilityTask_PlayTimeline::PlayTimeline(UGASGameplayAbility* OwningAbility, const FName TaskInstanceName, UCurveFloat* InCurveFloat, const float InStartTime,
	const float InPlayRate, const ETimelineDirection::Type InDirection, const bool bInDestroyOnEnd)
{
	if(!OwningAbility || !InCurveFloat)
	{
		if(!OwningAbility) {UE_LOG(LogTemp, Warning, TEXT("OwningAbility == NULL"));}
		else if(!InCurveFloat) {UE_LOG(LogTemp, Warning, TEXT("CurveFloat == NULL"));}
		return nullptr;
	}
	UAbilityTask_PlayTimeline* Task = NewAbilityTask<UAbilityTask_PlayTimeline>(OwningAbility, TaskInstanceName);
	Task->CurveFloat = InCurveFloat;
	Task->StartTime = InStartTime;
	Task->PlayRate = InPlayRate;
	Task->Direction = InDirection;
	Task->bDestroyOnEnd = bInDestroyOnEnd;
	return Task;
}

void UAbilityTask_PlayTimeline::Activate()
{
	Super::Activate();

	UGASGameplayAbility* GASAbility = CastChecked<UGASGameplayAbility>(Ability);
	GASAbility->OnFailedPrediction.AddDynamic(this, &UAbilityTask_PlayTimeline::FailedPrediction);
	GASAbility->OnGameplayAbilityEnded.AddUObject(this, &UAbilityTask_PlayTimeline::AbilityEnded);

	// Init start and end times
	CurveFloat->GetTimeRange(TimelineStartTime, TimelineEndTime);
	
	FOnTimelineFloatStatic Progress;
	Progress.BindUObject(this, &UAbilityTask_PlayTimeline::Progress);
	CurveFTimeline.AddInterpFloat(CurveFloat, Progress);

	FOnTimelineEvent EndEvent;
	EndEvent.BindDynamic(this, &UAbilityTask_PlayTimeline::End);
	
	CurveFTimeline.SetNewTime(StartTime);
	CurveFTimeline.SetPlayRate(PlayRate);
	
	if(Direction == ETimelineDirection::Forward)
	{
		CurveFTimeline.AddEvent(TimelineEndTime, EndEvent);
		bSetupEndTimelineEvent = true;
		
		CurveFTimeline.Play();
	}
	else if(Direction == ETimelineDirection::Backward)
	{
		CurveFTimeline.AddEvent(TimelineStartTime, EndEvent);
		bSetupStartTimelineEvent = true;
		
		CurveFTimeline.Reverse();
	}
}

void UAbilityTask_PlayTimeline::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	TaskDeltaTime = DeltaTime;
	CurveFTimeline.TickTimeline(DeltaTime);
}

void UAbilityTask_PlayTimeline::Progress(float Value)
{
	TimelineProgress.Broadcast(TaskDeltaTime, (TimelineStartTime + CurveFTimeline.GetPlaybackPosition()) / TimelineEndTime, Value);
}

void UAbilityTask_PlayTimeline::End()
{
	if(bDestroyOnEnd)
		EndTask();

	const float PlaybackPosition = CurveFTimeline.GetPlaybackPosition();
	TimelineEnd.Broadcast(0.f, (TimelineStartTime + PlaybackPosition) / PlaybackPosition, CurveFloat->GetFloatValue(CurveFTimeline.GetPlaybackPosition()));
}

void UAbilityTask_PlayTimeline::Reverse()
{
	if(Direction == ETimelineDirection::Forward)
	{
		if(!bSetupStartTimelineEvent)
		{
			FOnTimelineEvent TimelineEvent;
			TimelineEvent.BindDynamic(this, &UAbilityTask_PlayTimeline::End);
			CurveFTimeline.AddEvent(TimelineStartTime, TimelineEvent);
			bSetupStartTimelineEvent = true;
		}
		
		CurveFTimeline.Reverse();
		Direction = ETimelineDirection::Backward;
	}
	else if(Direction == ETimelineDirection::Backward)
	{
		if(!bSetupEndTimelineEvent)
		{
			FOnTimelineEvent TimelineEvent;
			TimelineEvent.BindDynamic(this, &UAbilityTask_PlayTimeline::End);
			CurveFTimeline.AddEvent(TimelineEndTime, TimelineEvent);
			bSetupEndTimelineEvent = true;
		}
		
		CurveFTimeline.Play();
		Direction = ETimelineDirection::Forward;
	}
}


void UAbilityTask_PlayTimeline::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);

	UGASGameplayAbility* GASAbility = CastChecked<UGASGameplayAbility>(Ability);
	GASAbility->OnFailedPrediction.RemoveAll(this);
	GASAbility->OnGameplayAbilityEnded.RemoveAll(this);
}






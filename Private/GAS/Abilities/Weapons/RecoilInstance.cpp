// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Weapons/RecoilInstance.h"

#include "Character/ShooterCharacter.h"
#include "Kismet/KismetMathLibrary.h"

int32 URecoilInstance::NumInstances = 0;

URecoilInstance::URecoilInstance()
{
	
}

URecoilInstance* URecoilInstance::AddRecoilInstance(AShooterCharacter* InWielder, const TSubclassOf<URecoilInstance>& RecoilClass, const float InRecoilMultiplier)
{
	if(!InWielder || !RecoilClass) return nullptr;

	if(URecoilInstance* Instance = NewObject<URecoilInstance>(GetTransientPackage(), RecoilClass))
	{
		// Prevent garbage collection
		Instance->AddToRoot();
		InWielder->OnDestroyed.AddDynamic(Instance, &URecoilInstance::WielderDestroyed);
		
		Instance->Wielder = InWielder;
		Instance->RecoilMultiplier = InRecoilMultiplier;
		Instance->Activate();
		
		return Instance;
	}
	return nullptr;
}

void URecoilInstance::Activate_Implementation()
{
	NumInstances++;
	
	FOnTimelineFloatStatic TimelineFloat;
	TimelineFloat.BindUObject(this, &URecoilInstance::TimelineProgress);
	RecoilTimeline.AddInterpFloat(RecoilCurve, TimelineFloat);
	RecoilTimeline.SetPlayRate(PlayRate);

	FOnTimelineEvent TimelineEvent;
	TimelineEvent.BindDynamic(this, &URecoilInstance::TimelineEnd);
	RecoilTimeline.AddEvent(RecoilTimeline.GetTimelineLength(), TimelineEvent);
 
	RecoilTimeline.PlayFromStart();

	TargetTransform = MakeTargetTransform();
	const float CalculatedMagnitude = CalculateTargetTransformMagnitude();
	TargetTransform.SetLocation(TargetTransform.GetLocation() * RecoilMultiplier * RecoilMagnitude * CalculatedMagnitude);
	TargetTransform.SetRotation(TargetTransform.GetRotation() * RecoilMultiplier * RecoilMagnitude * CalculatedMagnitude);
	
	TickerDelegate = FTickerDelegate::CreateUObject(this, &URecoilInstance::Tick);
	TickerDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickerDelegate);
}

FTransform URecoilInstance::MakeTargetTransform_Implementation() const
{
	return FTransform(FVector(0.f, 0.f, 10.f) * RecoilMultiplier * RecoilMagnitude);
}

bool URecoilInstance::Tick(const float DeltaTime)
{
	RecoilTimeline.TickTimeline(DeltaTime);
	
	return true;
}

void URecoilInstance::TimelineProgress_Implementation(const float Value)
{
	if(!Wielder)
	{
		Destroy();
		return;
	}
	
	const FTransform& NewTransform = UKismetMathLibrary::TLerp(FTransform::Identity, TargetTransform, Value);
	const FTransform& Difference = NewTransform.GetRelativeTransform(CurrentTransform);
	CurrentTransform = NewTransform;
	Wielder->FPOffsetTransform *= Difference;

	if(bApplyControllerRecoil)
	{
		const FRotator& RotationDifference = Difference.GetRotation().Rotator();
		Wielder->AddControllerPitchInput(-RotationDifference.Pitch * ControllerRecoilMultiplier);
		Wielder->AddControllerYawInput(RotationDifference.Yaw * ControllerRecoilMultiplier);
	}
}

void URecoilInstance::TimelineEnd_Implementation()
{
	// Allow garbage collection
	Destroy();
	
	NumInstances--;
	
	RecoilEnd.Broadcast(this);

	FTicker::GetCoreTicker().RemoveTicker(TickerDelegateHandle);
	TickerDelegate.Unbind();

	if(Wielder && NumInstances <= 0)
		Wielder->FPOffsetTransform = FTransform::Identity;

	PRINT(TEXT("Num Recoil Instances == %i"), NumInstances);
}






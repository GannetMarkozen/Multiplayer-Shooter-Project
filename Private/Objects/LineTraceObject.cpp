// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/LineTraceObject.h"

#include "DrawDebugHelpers.h"

FHitResult ULineTraceObject::DoLineTrace(const FVector& Location, const FRotator& Rotation, const TArray<AActor*>& IgnoreActors, float SpreadMagnitude, bool bDrawDebug)
{
	const FRotator& RandRotation = { Rotation.Pitch + FMath::FRandRange(-Spread.Y, Spread.Y), Rotation.Yaw + FMath::FRandRange(-Spread.X, Spread.X), 0.f };
	const FVector& End = Location + RandRotation.Vector() * Range;

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoreActors);
	GetWorld()->LineTraceSingleByChannel(Hit, Location, End, TraceChannel, QueryParams);
	if(bDrawDebug)
		DrawDebugLine(GetWorld(), Location, Hit.IsValidBlockingHit() ? Hit.Location : End, Hit.IsValidBlockingHit() ? FColor::Green : FColor::Red, false, 5.f);

	return Hit;
}

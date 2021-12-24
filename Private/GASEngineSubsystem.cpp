// Fill out your copyright notice in the Description page of Project Settings.


#include "GASEngineSubsystem.h"
#include "AbilitySystemGlobals.h"
#include "GAS/ExtendedTypes.h"
#include "MultiplayerShooter/MultiplayerShooter.h"

void UGASEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UAbilitySystemGlobals::Get().InitGlobalData();
}
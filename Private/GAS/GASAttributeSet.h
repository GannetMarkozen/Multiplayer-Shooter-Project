// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GASAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UGASAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UGASAttributeSet(){}

	// Uses the attribute to find the attribute data ptr
	virtual FGameplayAttributeData* FindAttributeData(const FGameplayAttribute& Attribute);

	UFUNCTION(BlueprintPure, Meta = (AutoCreateRefTerm = "Attribute", DisplayName = "Find Attribute Data"), Category = "Set")
	FORCEINLINE bool FindAttributeDataRef(const FGameplayAttribute& Attribute, FGameplayAttributeData& OutData)
	{
		if(const FGameplayAttributeData* Data = FindAttributeData(Attribute))
		{
			OutData = *Data;
			return true;
		}
		return false;
	}
};

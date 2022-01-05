// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "MultiplayerShooter.generated.h"

// Gets the size of multiple types
template<typename ... Types>
struct TSizeOf;
// Gets the size of multiple types
template<typename TFirst>
struct TSizeOf<TFirst>
{
	static constexpr int32 Size = sizeof(TFirst);
};
// Gets the size of multiple types
template<typename TFirst, typename... TRemaining>
struct TSizeOf<TFirst, TRemaining...>
{
	static constexpr int32 Size = sizeof(TFirst) + TSizeOf<TRemaining...>::Size;
};


template<typename ... Types>
struct TMemInit;

template<typename TFirst>
struct TMemInit<TFirst>
{
	static constexpr int32 Size = TSizeOf<TFirst>::Size;
	char Mem[Size];
	FORCEINLINE char* Get() { return Mem; }
};

template<typename TFirst, typename... TRemaining>
struct TMemInit<TFirst, TRemaining...>
{
	static constexpr int32 Size = TSizeOf<TFirst, TRemaining...>::Size;
	char Mem[Size];
	FORCEINLINE char* Get() { return Mem; }
};

// Defines the input bound in a gameplay ability
UENUM(BlueprintType)
enum class EAbilityInput : uint8
{
	None,
	Confirm,
	Cancel,

	Jump,
	PrimaryFire,
	SecondaryFire,
	Interact,
	Reload,
	SwapToLastItem,
	MWheelUp,
	MWheelDown,
	MWheelPressed,
};

UCLASS()
class MULTIPLAYERSHOOTER_API UMultiplayerShooterFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FORCEINLINE FGameplayTagContainer CreateTagContainer(const FGameplayTag& Tag) { return FGameplayTagContainer(Tag); }
	static FORCEINLINE FGameplayTagContainer CreateTagContainer(const FName& TagName) { return FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TagName)); }
	static FORCEINLINE FGameplayTagContainer CreateTagContainer(const TArray<FGameplayTag>& Tags)
	{
		FGameplayTagContainer Container;
		for(const FGameplayTag& Tag : Tags)
			Container.AddTag(Tag);
		return Container;
	}
	static FORCEINLINE FGameplayTagContainer CreateTagContainer(const TArray<FName>& TagNames)
	{
		FGameplayTagContainer Container;
		for(const FName& TagName : TagNames)
			Container.AddTag(FGameplayTag::RequestGameplayTag(TagName));
		return Container;
	}
	static FORCEINLINE FGameplayTagContainer CreateTagContainer(const std::initializer_list<FName>& TagNames)
	{
		FGameplayTagContainer Container;
		for(const FName& TagName : TagNames)
			Container.AddTag(FGameplayTag::RequestGameplayTag(TagName));
		return Container;
	}

	static FORCEINLINE FGameplayTagContainer CreateTagContainer(const TArray<FGameplayTagContainer>& Containers)
	{
		FGameplayTagContainer OutContainer;
		for(const FGameplayTagContainer& Container : Containers)
			OutContainer.AppendTags(Container);
		return OutContainer;
	}

	UFUNCTION(BlueprintCallable)
	static FORCEINLINE FString BoolToString(const bool bCheck) { return bCheck ? "true" : "false"; }

	UFUNCTION(BlueprintCallable)
	static FORCEINLINE FString AuthToString(const bool bAuth) { return bAuth ? "Server" : "Client"; }
};

#define BOOLTOSTRING(bCheck) FString(bCheck ? "True" : "False")

#define AUTHTOSTRING(bAuth) FString(bAuth ? "Server" : "Client")

#define PRINT(...) if(GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::White, FString::Printf(##__VA_ARGS__))

#define PRINTCOLOR(Color, ...) if(GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, Color, FString::Printf(##__VA_ARGS__))

#define PRINTLINE PRINT(TEXT("%s"), *FString(__FUNCTION__))

#define TAG(TagName) FGameplayTag::RequestGameplayTag(FName(TagName))

#define TAG_CONTAINER(...) UMultiplayerShooterFunctionLibrary::CreateTagContainer(__VA_ARGS__)
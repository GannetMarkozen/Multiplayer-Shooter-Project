// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "EffectVolume.generated.h"



USTRUCT(BlueprintType)
struct FEffectLevelPair
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayEffect> Effect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;
};

/** An actor initialized with funcitons
*	to apply an effect for the duration
*	a character is overlapping the
*	rootcomponent. Must be a primitive
*	component.
*/

UCLASS()
class MULTIPLAYERSHOOTER_API AEffectVolume : public AActor
{
	GENERATED_BODY()
public:
	AEffectVolume();
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void BeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Effect Volume")
	void CharacterBeginOverlap(class AShooterCharacter* Character);
	virtual void CharacterBeginOverlap_Implementation(class AShooterCharacter* Character);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Effect Volume")
	void CharacterEndOverlap(class AShooterCharacter* Character);
	virtual void CharacterEndOverlap_Implementation(class AShooterCharacter* Character);

	
	// The active effect handle with the associated character
	UPROPERTY(VisibleInstanceOnly, Category = "State")
	TMap<class AShooterCharacter*, FActiveGameplayEffectHandle> ActiveEffectHandles;

	// The period timer handle with the associated character. This solves the issue of the
	// character going back and out through the effect volume and applying damage too quickly.
	// This ensures the period time has ended before being applied to the character again
	UPROPERTY(VisibleInstanceOnly, Category = "State")
	TMap<class AShooterCharacter*, FTimerHandle> EndPeriodTimerHandles;
	
	// The effect to be applied for the duration the character is within in the effect volume
	UPROPERTY(EditAnywhere, Category = "Configurations")
	TSubclassOf<class UGameplayEffect> EffectClass;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	float Level = 1.f;

	// Casted primitive root component
	UPROPERTY(BlueprintReadWrite, Meta = (DisplayName = "Primitive Root Component"), Category = "State")
	TObjectPtr<class UPrimitiveComponent> OverlapComp;

	// Override to have the overlap component not be the root comp. Must be primitive component
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	FName OverlapCompName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Configurations")
	bool bApplyOnlyOnServer = true;
};

UCLASS()
class MULTIPLAYERSHOOTER_API AEffectBoxVolume : public AEffectVolume
{
	GENERATED_BODY()
public:
	AEffectBoxVolume();
protected:
	// Initialized with box component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class UBoxComponent> BoxComponent;
};


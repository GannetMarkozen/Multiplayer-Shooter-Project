#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags/Public/GameplayTags.h"

#include "Item.generated.h"

UCLASS(Abstract)
class MULTIPLAYERSHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
public:
	AItem();
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	TObjectPtr<class UTexture2D> Thumbnail;
	
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "Item"), Category = "Configurations")
	FGameplayTag ItemTag;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, Category = "Configurations")
	FText Description;
	
public:	
	UFUNCTION(BlueprintPure, Category = "Item")
	const FORCEINLINE FGameplayTag& GetItemTag() const { return ItemTag; }

	// Interaction that has to do with UI. Not 3D space interaction. That is dealt with the
	// Interact Interface on the item pickup class
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void Interact(class AShooterCharacter* Interactor);
	virtual FORCEINLINE void Interact_Implementation(class AShooterCharacter* Interactor) { UE_LOG(LogTemp, Warning, TEXT("No Interact implementation on %s"), *GetName()); }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	void EndInteract(class AShooterCharacter* Interactor);
	virtual FORCEINLINE void EndInteract_Implementation(class AShooterCharacter* Interactor) { UE_LOG(LogTemp, Warning, TEXT("No EndInteract implementation on %s"), *GetName()); }

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void Inspect(class AShooterCharacter* Interactor);
	virtual FORCEINLINE void Inspect_Implementation(class AShooterCharacter* Interactor) { UE_LOG(LogTemp, Warning, TEXT("No Inspect implementation on %s"), *GetName()); }

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void EndInspect(class AShooterCharacter* Interactor);
	virtual FORCEINLINE void EndInspect_Implementation(class AShooterCharacter* Interactor) { UE_LOG(LogTemp, Warning, TEXT("No EndInspect implementation on %s"), *GetName()); }
	
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE class UTexture2D* GetThumbnail() const { return Thumbnail; }

	UFUNCTION(BlueprintPure, Category = "Item")
	const FORCEINLINE FText& GetItemName() const { return ItemName; }

	UFUNCTION(BlueprintPure, Category = "Item")
	const FORCEINLINE FText& GetDescription() const { return Description; }
};
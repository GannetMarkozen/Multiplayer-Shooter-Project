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
	
	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void Interact();
	virtual FORCEINLINE void Interact_Implementation() { UE_LOG(LogTemp, Warning, TEXT("No Interact implementation on %s"), *GetName()); }

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void Inspect();
	virtual FORCEINLINE void Inspect_Implementation() { UE_LOG(LogTemp, Warning, TEXT("No Inspect implementation on %s"), *GetName()); }

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void EndInspect();
	virtual FORCEINLINE void EndInspect_Implementation() { UE_LOG(LogTemp, Warning, TEXT("No EndInspect implementation on %s"), *GetName()); }
	
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE class UTexture2D* GetThumbnail() const { return Thumbnail; }

	UFUNCTION(BlueprintPure, Category = "Item")
	const FORCEINLINE FText& GetItemName() const { return ItemName; }

	UFUNCTION(BlueprintPure, Category = "Item")
	const FORCEINLINE FText& GetDescription() const { return Description; }
};
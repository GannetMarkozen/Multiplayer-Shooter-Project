#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "DamageInterface.generated.h"


UINTERFACE(MinimalAPI)
class UDamageInterface : public UInterface
{
	GENERATED_BODY()	
};

class IDamageInterface
{
	GENERATED_BODY()
public:
	// Static: Only call after checking if target implements the interface. Okay to call this directly in C++
	static void ApplyDamage(class UAbilitySystemComponent* Instigator, class AActor* Target, const FGameplayEffectSpecHandle& Spec)
	{
		if(!Instigator || !Target || !Target->GetClass()->ImplementsInterface(UDamageInterface::StaticClass())) return;
		if(const IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(Target))
		{
			Instigator->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetInterface->GetAbilitySystemComponent());
		}

		IDamageInterface::Execute_ReceivedDamage(Target, Instigator->GetOwner(), Spec);
	}
	
	// Static: Only call after checking if target implements the interface. Okay to call this directly in C++
	static void ApplyDamage(class AActor* Instigator, class AActor* Target, const FGameplayEffectSpecHandle& Spec)
	{
		if(const IAbilitySystemInterface* InstigatorInterface = Cast<IAbilitySystemInterface>(Instigator))
			ApplyDamage(InstigatorInterface->GetAbilitySystemComponent(), Target, Spec);
	}
	
protected:
	// Called whenever receiving damage, event on AActors without an ASC
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ReceivedDamage(class AActor* Instigator, const FGameplayEffectSpecHandle& Spec);
	virtual FORCEINLINE void ReceivedDamage_Implementation(class AActor* Instigator, const FGameplayEffectSpecHandle& Spec) {}
};

UCLASS()
class UDamageInterfaceFuncs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Call to apply damage dynamically to actors possessing the IDamageInterface. Use this implementation to avoid casting if possible
	UFUNCTION(BlueprintCallable, Category = "GAS")
	static FORCEINLINE void ApplyDamageFromASC(class UAbilitySystemComponent* Instigator, class AActor* Target, const FGameplayEffectSpecHandle& Spec)
	{
		IDamageInterface::ApplyDamage(Instigator, Target, Spec);
	}

	UFUNCTION(BlueprintCallable, Category = "GAS")
	static FORCEINLINE void ApplyDamage(class AActor* Instigator, class AActor* Target, const FGameplayEffectSpecHandle& Spec)
	{
		IDamageInterface::ApplyDamage(Instigator, Target, Spec);
	}
};
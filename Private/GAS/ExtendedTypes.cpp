#include "GAS/ExtendedTypes.h"

#include "GameplayCueSet.h"
#include "Character/ShooterCharacter.h"
#include "GAS/GASGameplayCueNotify_Static.h"

UAbilitySystemGlobalsExtended::UAbilitySystemGlobalsExtended()
{
	
}

UAbilitySystemGlobalsExtended& UAbilitySystemGlobalsExtended::Get()
{
	return (UAbilitySystemGlobalsExtended&)UAbilitySystemGlobals::Get();
}

UGameplayCueManagerExtended* UAbilitySystemGlobalsExtended::GetGameplayCueManagerExtended()
{
	return (UGameplayCueManagerExtended*)GetGameplayCueManager();
}


void FGameplayAbilityActorInfoExtended::InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	Super::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);
	if(InAvatarActor)
	{
		Character = CastChecked<AShooterCharacter>(InAvatarActor);
		if(Character.IsValid())
		{
			Inventory = Character.Get()->GetCharacterInventory();
			ASC = Character.Get()->GetASC();
		}
	}
}

void UGameplayCueManagerExtended::InvokeGameplayCueExecuted_FromSpec(UAbilitySystemComponent* OwningComponent, const FGameplayEffectSpec& Spec, FPredictionKey PredictionKey)
{
	//Super::InvokeGameplayCueExecuted_FromSpec(OwningComponent, Spec, PredictionKey);
	for(const FGameplayEffectCue& EffectCue : Spec.Def->GameplayCues)
	{
		for(TArray<FGameplayTag>::TConstIterator TagItr(EffectCue.GameplayCueTags.CreateConstIterator()); TagItr; ++TagItr)
		{
			if(const UClass* NotifyClass = GetGameplayCueNotifyClass(*TagItr))
			{
				if(const UGASGameplayCueNotify_Static* Notify = Cast<UGASGameplayCueNotify_Static>(NotifyClass->GetDefaultObject()))
				{
					// Init params
					FGameplayCueParameters Params;
					UAbilitySystemGlobalsExtended::Get().InitGameplayCueParameters_GESpec(Params, Spec);

					// Determine net execution policy
					switch(Notify->GetNetExecutionPolicy())
					{
						case EGameplayCueNetExecutionPolicy::NetMulticast:
							Super::InvokeGameplayCueExecuted_FromSpec(OwningComponent, Spec, PredictionKey);
							break;
						case EGameplayCueNetExecutionPolicy::LocalTarget:
							CastChecked<UGASAbilitySystemComponent>(OwningComponent)->ExecuteGameplayCueLocal(*TagItr, Params);
							break;
						case EGameplayCueNetExecutionPolicy::LocalInstigator:
							CastChecked<UGASAbilitySystemComponent>(Spec.GetContext().Get()->GetInstigatorAbilitySystemComponent())->ExecuteGameplayCueLocal(*TagItr, Params);
							break;
					}
				}
				else UE_LOG(LogTemp, Error, TEXT("%s: Failed cast to UGASGameplayCueNotify_Static"), *FString(__FUNCTION__));
			}
		}
	}
}


UClass* UGameplayCueManagerExtended::GetGameplayCueNotifyClass(const FGameplayTag& GameplayCueTag)
{
	if(const int32* Index = GetRuntimeCueSet()->GameplayCueDataMap.Find(GameplayCueTag))
	{
		if(*Index != INDEX_NONE && GetRuntimeCueSet()->GameplayCueData.IsValidIndex(*Index))
		{
			if(UClass* NotifyClass = FindObject<UClass>(nullptr, *GetRuntimeCueSet()->GameplayCueData[*Index].GameplayCueNotifyObj.ToString()))
			{
				return NotifyClass;
			}
			else UE_LOG(LogTemp, Error, TEXT("%s: No notify class"), *FString(__FUNCTION__));
		}
		else UE_LOG(LogTemp, Error, TEXT("%s: Invalid index"), *FString(__FUNCTION__))
	}
	else UE_LOG(LogTemp, Error, TEXT("%s: Cannot find index"), *FString(__FUNCTION__));

	return nullptr;
}


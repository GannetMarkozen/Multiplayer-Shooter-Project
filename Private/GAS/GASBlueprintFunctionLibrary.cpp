﻿#include "GAS/GASBlueprintFunctionLibrary.h"

#include "Character/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

int32 UGASBlueprintFunctionLibrary::CalculateDamage(AActor* Target, AActor* Instigator, const FGameplayEffectSpecHandle& Spec, const bool bDisplayDamage)
{
	if(!Spec.IsValid() || !Target || !Instigator) return 0;
	if(const AShooterCharacter* InstigatorCharacter = Cast<AShooterCharacter>(Instigator))
	{
		if(const AWeapon* Weapon = InstigatorCharacter->GetCharacterInventory()->GetCurrent())
		{
			const int32 Damage = IDamageCalculationInterface::Execute_CalculateDamage(Weapon, Target, Spec);
			if(bDisplayDamage)
			{
				((FGameplayEffectContextExtended*)Spec.Data.Get()->GetEffectContext().Get())->SetTarget(Target);
				FGameplayCueParameters Params;
				Params.Instigator = Instigator;
				Params.EffectContext = Spec.Data.Get()->GetEffectContext();
				Params.RawMagnitude = -Damage;
				InstigatorCharacter->GetASC()->NetMulticast_InvokeGameplayCueExecuted_WithParams(TAG("GameplayCue.Damage"), FPredictionKey(), Params);
			}
			return Damage;
		}
	}
	else if(const float* BaseDamage = Spec.Data.Get()->SetByCallerTagMagnitudes.Find(UAbilitySystemGlobalsExtended::Get().GetBaseDamageTag()))
	{
		return *BaseDamage;
	}
	return 0;
}

void UGASBlueprintFunctionLibrary::ApplyDamageKnockback(const FGameplayEffectContextHandle& Context, float Damage, float KnockbackMagnitude)
{
	const FGameplayAbilityTargetDataHandle& Handle = Extend(Context.Get())->GetTargetData();
	for(int32 i = 0; i < Handle.Data.Num(); i++)
	{
		if(!Handle.Data[i].Get()) continue;
		if(Handle.Data[i].Get()->HasHitResult())
		{
			const FHitResult& Hit = *Handle.Data[i].Get()->GetHitResult();
			if(!(!Extend(Context.Get())->GetTarget() || Extend(Context.Get())->GetTarget() == Hit.GetActor())) continue;

			const AShooterCharacter* HitChar = Cast<AShooterCharacter>(Hit.GetActor());
			if(HitChar && !HitChar->GetASC()->HasMatchingGameplayTag(UAbilitySystemGlobalsExtended::Get().GetDeadTag())) 
			{// If hit shooter character and is not dead, apply character knockback
				const FVector& Impulse = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal() * (HitChar->GetCharacterMovement()->IsFalling() ? 0.3f : 1.f) * Damage * KnockbackMagnitude * 100.f;
				HitChar->GetCharacterMovement()->Launch(Impulse);
			}
			else if(Hit.GetComponent())
			{
				const ECollisionChannel ObjType = Hit.GetComponent()->GetCollisionObjectType();
				if(Hit.GetComponent()->IsSimulatingPhysics(Hit.BoneName) && (ObjType == ECC_Pawn || ObjType == ECC_PhysicsBody))
				{// Else if a simulating component, apply generic knockback
					const FVector& Impulse = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal() * Damage * KnockbackMagnitude * 1000.f;
					Hit.GetComponent()->AddImpulseAtLocation(Impulse, Hit.Location, Hit.BoneName);
				}
			}
		}
	}
}
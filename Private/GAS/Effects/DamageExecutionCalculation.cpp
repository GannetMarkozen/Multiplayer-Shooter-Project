// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DamageExecutionCalculation.h"

#include "Character/ShooterCharacter.h"
#include "GAS/ExtendedTypes.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"
#include "GAS/GASBlueprintFunctionLibrary.h"
#include "MultiplayerShooter/MultiplayerShooter.h"

struct DamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(BulletResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ExplosionResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);

	DamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCharacterAttributeSet, BulletResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCharacterAttributeSet, ExplosionResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UCharacterAttributeSet, Health, Target, false);
	}
};

static const DamageStatics& GetDamageStatics()
{
	static DamageStatics Statics;
	return Statics;
}

UDamageExecutionCalculation::UDamageExecutionCalculation()
{
	DefaultHeadshotMultiplier = 1.5f;

	// Get resistances of target for calculation
	RelevantAttributesToCapture.Add(GetDamageStatics().BulletResistanceDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().ExplosionResistanceDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().HealthDef);
}

void UDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	const FGameplayTagContainer* SourceTags = ExecutionParams.GetOwningSpec().CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = ExecutionParams.GetOwningSpec().CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParams;
	EvaluationParams.SourceTags = SourceTags;
	EvaluationParams.TargetTags = TargetTags;

	float Damage = CalculateBaseDamage(ExecutionParams, EvaluationParams);
	if(Damage == 0.f) return;
	
	FGameplayTagContainer AssetTags;
	ExecutionParams.GetOwningSpec().GetAllAssetTags(AssetTags);
	
	// Multiply damage by number of hits, useful for shotguns
	TArray<const FHitResult*> Hits;
	GetHits(Hits, ExecutionParams);
	
	//float Damage = BaseDamage * static_cast<float>(Hits.Num());

	// If can headshot, add all headshot multipliers on headshots
	if(AssetTags.HasTagExact(TAG("Data.CanHeadshot")))
	{
		int32 NumHeadshots = 0;
		for(const FHitResult* Hit : Hits)
		{
			if(!Hit) continue;
			if(Hit->BoneName == FName("head"))
				NumHeadshots++;
		}
		const float* CustomHeadshotMultiplier = ExecutionParams.GetOwningSpec().SetByCallerTagMagnitudes.Find(TAG("Data.HeadshotMultiplier"));
		Damage += ((CustomHeadshotMultiplier ? *CustomHeadshotMultiplier : DefaultHeadshotMultiplier) - 1.f) * NumHeadshots * (Damage / Hits.Num());
	}

	// Set effect context target to the affected actor for gameplay cues
	((FGameplayEffectContextExtended*)ExecutionParams.GetOwningSpec().GetEffectContext().Get())->SetTarget(ExecutionParams.GetTargetAbilitySystemComponent()->GetOwner());
	if(Damage > 0.f)
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UCharacterAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -Damage));
}

float UDamageExecutionCalculation::CalculateBaseDamage(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FAggregatorEvaluateParameters& EvaluationParams) const
{
	float BaseDamage = IDamageCalculationInterface::Execute_CalculateDamage(
		((FGameplayAbilityActorInfoExtended&)*ExecutionParams.GetOwningSpec().GetEffectContext().Get()->GetInstigatorAbilitySystemComponent()->AbilityActorInfo.Get()).Inventory.Get()->GetCurrentWeapon(),
		ExecutionParams.GetTargetAbilitySystemComponent()->GetOwner(), FGameplayEffectSpecHandle(new FGameplayEffectSpec(ExecutionParams.GetOwningSpec())));
	if(BaseDamage <= 0.f) return 0.f;

	FGameplayTagContainer AssetTags;
	ExecutionParams.GetOwningSpec().GetAllAssetTags(AssetTags);

	// Determine damage type then apply target resistances to damage
	if(AssetTags.HasTagExact(TAG("Data.DamageType.Bullet")))
	{
		float BulletResistance = 0.f;
		if(ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().BulletResistanceDef, EvaluationParams, BulletResistance))
			BaseDamage *= 1.f - (BulletResistance / 100.f);
	}
	else if(AssetTags.HasTagExact(TAG("Data.DamageType.Explosion")))
	{
		float ExplosionResistance = 0.f;
		if(ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().ExplosionResistanceDef, EvaluationParams, ExplosionResistance))
			BaseDamage *= 1.f - (ExplosionResistance / 100.f);
	}

	return BaseDamage;
}

void UDamageExecutionCalculation::GetHits(TArray<const FHitResult*>& OutHits, const FGameplayEffectCustomExecutionParameters& ExecutionParams) const
{
	const FGameplayAbilityTargetDataHandle& Handle = ((FGameplayEffectContextExtended*)ExecutionParams.GetOwningSpec().GetEffectContext().Get())->GetTargetData();
	if(!Handle.Data.IsEmpty())
	{// If has target data, return hits from target data
		for(int32 i = 0; i < Handle.Data.Num(); i++)
		{
			if(Handle.IsValid(i))
			{
				if(const FHitResult* Hit = Handle.Data[i].Get()->GetHitResult())
				{// If valid hit
					if(Hit->GetActor() == ExecutionParams.GetTargetAbilitySystemComponent()->GetOwner())
					{// If hit is this effect's target
						OutHits.Add(Hit);
					}
				}
			}
		}
	}
	else if(const FHitResult* Hit = ExecutionParams.GetOwningSpec().GetEffectContext().GetHitResult())
	{// else if context has hit
		if(Hit->GetActor() == ExecutionParams.GetTargetAbilitySystemComponent()->GetOwner())
		{// If hit is this effect's target
			OutHits.Add(Hit);
		}
	}
}



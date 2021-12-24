// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DamageExecutionCalculation.h"

#include "GAS/ExtendedTypes.h"
#include "GAS/GASAttributeSet.h"
#include "MultiplayerShooter/MultiplayerShooter.h"

struct DamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(BulletResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ExplosionResistance);

	DamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, BulletResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAttributeSet, ExplosionResistance, Target, false);
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
}

void UDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParams;
	EvaluationParams.SourceTags = SourceTags;
	EvaluationParams.TargetTags = TargetTags;
	
	const float* GetBaseDamage = ExecutionParams.GetOwningSpec().SetByCallerTagMagnitudes.Find(FGameplayTag::RequestGameplayTag(FName("Data.BaseDamage")));
	float BaseDamage = GetBaseDamage ? *GetBaseDamage : 0.f;
	if(BaseDamage <= 0.f) return;

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

	// If can headshot is true, see if hit result returns head hit then apply headshot multiplier
	/*
	if(AssetTags.HasTagExact(TAG("Data.CanHeadshot")))
	{
		const FHitResult* Hit = nullptr;
		
		if(const FGameplayAbilityTargetData* TargetData = static_cast<FGameplayEffectContextExtended*>(Spec.GetContext().Get())->GetTargetData().Get(0))
		{// get hit from target data first index if exists
			Hit = TargetData->GetHitResult();
		}
		
		if(!Hit)
		{// else get hit from effect context
			Hit = Spec.GetContext().Get()->GetHitResult();
		}
		
		if(Hit && Hit->BoneName == FName("b_head"))
		{// Check if custom headshot multiplier is valid, else use default headshot multiplier
			const float* CustomHeadshotMultiplier = Spec.SetByCallerTagMagnitudes.Find(TAG("Data.HeadshotMultiplier"));
			Damage *= CustomHeadshotMultiplier ? *CustomHeadshotMultiplier : DefaultHeadshotMultiplier;
		}
	}*/

	// Multiply damage by number of hits, useful for shotguns
	TArray<const FHitResult*> Hits;
	GetHits(Hits, ExecutionParams);
	float Damage = BaseDamage * static_cast<float>(Hits.Num());

	// If can headshot, add all headshot multipliers on headshots
	if(AssetTags.HasTagExact(TAG("Data.CanHeadshot")))
	{
		int32 NumHeadshots = 0;
		for(const FHitResult* Hit : Hits)
		{
			if(!Hit) continue;
			if(Hit->BoneName == FName("b_head"))
				NumHeadshots++;
		}
		const float* CustomHeadshotMultiplier = Spec.SetByCallerTagMagnitudes.Find(TAG("Data.HeadshotMultiplier"));
		Damage += ((CustomHeadshotMultiplier ? *CustomHeadshotMultiplier : DefaultHeadshotMultiplier) - 1.f) * static_cast<float>(NumHeadshots) * BaseDamage;
		PRINT(TEXT("NumHeadshots == %i"), NumHeadshots);
	}

	PRINT(TEXT("Damage == %f"), Damage);
	
	if(Damage > 0.f)
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UGASAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -Damage));
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



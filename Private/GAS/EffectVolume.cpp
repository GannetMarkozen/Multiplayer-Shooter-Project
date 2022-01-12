// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/EffectVolume.h"
#include "Character/ShooterCharacter.h"
#include "Components/BoxComponent.h"

AEffectVolume::AEffectVolume()
{
	
}

void AEffectVolume::BeginPlay()
{
	Super::BeginPlay();
	
	if(bApplyOnlyOnServer && !HasAuthority()) return;
	if(OverlapCompName != NAME_None)
	{
		TArray<UPrimitiveComponent*> Comps;
		GetComponents<UPrimitiveComponent>(Comps);
		for(UPrimitiveComponent* Comp : Comps)
			if(Comp->GetFName() == OverlapCompName)
			{
				OverlapComp = Comp;
				break;
			}
	}
	else
	{
		OverlapComp = Cast<UPrimitiveComponent>(RootComponent);
	}
			
	if(OverlapComp)
	{
		OverlapComp->OnComponentBeginOverlap.AddDynamic(this, &AEffectVolume::BeginOverlap);
		OverlapComp->OnComponentEndOverlap.AddDynamic(this, &AEffectVolume::EndOverlap);
	}
}



void AEffectVolume::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor))
		if(!ActiveEffectHandles.Find(Character) && !EndPeriodTimerHandles.Find(Character))
			CharacterBeginOverlap(Character);
}

void AEffectVolume::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor))
		if(!OverlapComp->IsOverlappingActor(Character))
			CharacterEndOverlap(Character);
}

void AEffectVolume::CharacterBeginOverlap_Implementation(AShooterCharacter* Character)
{
	if(!HasAuthority() && !Character->IsLocallyControlled()) return;
	const FGameplayEffectContextHandle& Context = Character->GetASC()->MakeEffectContextExtended(Character);
	const FGameplayEffectSpecHandle& Spec = Character->GetASC()->MakeOutgoingSpec(EffectClass, Level, Context);
	if(Spec.IsValid())
	{
		const FActiveGameplayEffectHandle& ActiveHandle = Character->GetASC()->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		ActiveEffectHandles.Add(Character, ActiveHandle);
	}
}

void AEffectVolume::CharacterEndOverlap_Implementation(AShooterCharacter* Character)
{
	if(!HasAuthority() && !Character->IsLocallyControlled()) return;
	if(const FActiveGameplayEffectHandle* ActiveHandle = ActiveEffectHandles.Find(Character))
	{
		if(const FActiveGameplayEffect* ActiveEffect = Character->GetASC()->GetActiveGameplayEffect(*ActiveHandle))
		{
			const float TimeRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(ActiveEffect->PeriodHandle);
			if(TimeRemaining > 0.05f)
			{
				FTimerHandle EndPeriodTimerHandle;
				EndPeriodTimerHandles.Add(Character, EndPeriodTimerHandle);
				const auto& EndPeriod = [=]()->void
				{
					EndPeriodTimerHandles.Remove(Character);
					if(IsValid(Character) && CastChecked<UPrimitiveComponent>(RootComponent)->IsOverlappingActor(Character))
						CharacterBeginOverlap(Character);
				};
				GetWorldTimerManager().SetTimer(EndPeriodTimerHandle, EndPeriod, TimeRemaining, false);
			}
		}
		
		Character->GetASC()->RemoveActiveGameplayEffect(*ActiveHandle);
		ActiveEffectHandles.Remove(Character);
	}
}




AEffectBoxVolume::AEffectBoxVolume()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionObjectType(ECC_WorldStatic);
	BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxComponent->SetCollisionResponseToChannel(ECC_Projectile, ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECC_ItemDrop, ECR_Ignore);
	BoxComponent->SetMobility(EComponentMobility::Static);
	RootComponent = BoxComponent;
}








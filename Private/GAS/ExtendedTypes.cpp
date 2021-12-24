#include "GAS/ExtendedTypes.h"
#include "Character/ShooterCharacter.h"

UAbilitySystemGlobalsExtended::UAbilitySystemGlobalsExtended()
{
	
}

UAbilitySystemGlobalsExtended& UAbilitySystemGlobalsExtended::Get()
{
	return (UAbilitySystemGlobalsExtended&)UAbilitySystemGlobals::Get();
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

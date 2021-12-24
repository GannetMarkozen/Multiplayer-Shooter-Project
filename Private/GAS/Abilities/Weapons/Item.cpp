#include "MultiplayerShooter/Public/GAS/Abilities/Weapons/Item.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
}
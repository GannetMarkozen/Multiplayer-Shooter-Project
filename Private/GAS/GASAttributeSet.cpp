// Fill out your copyright notice in the Description page of Project Settings.


#include "GASAttributeSet.h"


FGameplayAttributeData* UGASAttributeSet::FindAttributeData(const FGameplayAttribute& Attribute)
{
	for(TFieldIterator<FStructProperty> Itr(GetClass()); Itr; ++Itr)
		if(Itr->GetName() == Attribute.GetName())
			return Itr->ContainerPtrToValuePtr<FGameplayAttributeData>(this);
			
	return nullptr;
}

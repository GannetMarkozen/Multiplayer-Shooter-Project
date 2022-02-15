#include "GAS/Abilities/Weapons/InputBinding.h"

template <typename UserClass, typename ReturnType, typename ... ParamTypes, typename EnumType>
void UInputBindingHandler::AddBindUObject(APawn* Pawn, UserClass* UserObject, ReturnType(UserClass::* MemFuncPtr)(ParamTypes...), const EnumType EnumValue)
{
	UInputBinding::BindInputUObject(Pawn, UserObject, MemFuncPtr, EnumValue);
	ActiveCPP_Binds.Add(MemFuncPtr);
}

template <typename Func, typename ... VarTypes, typename EnumType>
void UInputBindingHandler::AddBindLambda(APawn* Pawn, UObject* UserObject, const Func&& FuncPtr, const EnumType EnumValue, VarTypes... Vars)
{
	UInputBinding::BindInputLambda(Pawn, UserObject, FuncPtr, EnumValue);
	ActiveCPP_Binds.Add(FuncPtr);
}

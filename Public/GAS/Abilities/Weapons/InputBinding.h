#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputBinding.generated.h"


UENUM(BlueprintType)
enum class EInputBinding : uint8
{
	PrimaryFire			UMETA(DisplayName = "Primary-Fire"),
	SecondaryFire		UMETA(DisplayName = "Secondary-Fire"),
	Jump				UMETA(DisplayName = "Jump"),
};

UCLASS()
class MULTIPLAYERSHOOTER_API UInputBinding : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:	
	// Gets the value name of the passed in enum
	template<typename EnumType>
	static FORCEINLINE FString EnumValueToString(const EnumType EnumValue)
	{
		static_assert(TIsEnum<EnumType>::Value, "UEnumHelpers::EnumValueToString() Must input enum value");
		return StaticEnum<EnumType>()->GetNameStringByIndex((int32)EnumValue);
	}

	template<typename EnumType>
	static FORCEINLINE FName GetEnumValueName(const EnumType EnumValue)
	{
		return FName(UInputBinding::EnumValueToString<EnumType>(EnumValue));
	}

	template<typename EnumType>
	static FORCEINLINE bool HasBind(class APawn* Pawn, UObject* UserObject, const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed)
	{
		if(!Pawn || !Pawn->Controller || !Pawn->Controller->InputComponent || !UserObject) return false;
		const UInputComponent* InputComponent = Pawn->Controller->InputComponent;
		for(int32 i = 0; i < InputComponent->GetNumActionBindings(); i++)
		{
			const FInputActionBinding& Binding = InputComponent->GetActionBinding(i);
			if(Binding.GetActionName() == UInputBinding::GetEnumValueName<EnumType>(EnumValue) &&
				Binding.KeyEvent == InputEvent &&
					Binding.ActionDelegate.GetUObject() == UserObject)
						return true;
		}
		return false;
	}

	template<typename EnumType>
	static FORCEINLINE void BindInputUFunction(class APawn* Pawn, class UObject* UserObject, const FName& FunctionName, const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed)
	{
		if(!Pawn || !Pawn->Controller || !Pawn->Controller->InputComponent || !UserObject) return;
		FInputActionBinding Binding(UInputBinding::GetEnumValueName<EnumType>(EnumValue), InputEvent);
		Binding.ActionDelegate.BindDelegate(UserObject, FunctionName);
		Binding.bConsumeInput = false;
		Pawn->Controller->InputComponent->AddActionBinding(Binding);
	}

	// Bind object member function ptr
	template<typename EnumType, typename UserClass, typename ReturnType, typename... ParamTypes>
	static FORCEINLINE void BindInputUObject(class APawn* Pawn, UserClass* UserObject, ReturnType(UserClass::* MemFuncPtr)(ParamTypes...), const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed)
	{
		if(!Pawn || !Pawn->Controller || !Pawn->Controller->InputComponent || !UserObject) return;
		FInputActionBinding Binding(UInputBinding::GetEnumValueName<EnumType>(EnumValue), InputEvent);
		Binding.ActionDelegate.GetDelegateForManualSet().BindUObject(UserObject, MemFuncPtr);
		Binding.bConsumeInput = false;
		Pawn->Controller->InputComponent->AddActionBinding(Binding);
	}

	template<typename EnumType, typename Func, typename ... VarTypes>
	static FORCEINLINE void BindInputLambda(class APawn* Pawn, UObject* UserObject, Func&& FuncPtr, const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed, VarTypes... Vars)
	{
		if(!Pawn || !Pawn->Controller || !Pawn->Controller->InputComponent || !UserObject) return;
		FInputActionBinding Binding(UInputBinding::GetEnumValueName<EnumType>(EnumValue), InputEvent);
		Binding.ActionDelegate.GetDelegateForManualSet().BindLambda(FuncPtr, Vars...);
		Binding.bConsumeInput = false;
		Pawn->Controller->InputComponent->AddActionBinding(Binding);
	}

	// Removes all bound input from object
	template<typename EnumType>
	static FORCEINLINE void RemoveInputUObject(class APawn* Pawn, class UObject* UserObject, const EnumType EnumValue, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed)
	{
		if(!Pawn || !Pawn->Controller || !Pawn->Controller->InputComponent || !UserObject) return;
		UInputComponent* InputComponent = Pawn->Controller->InputComponent;
		for(int32 i = 0; i < InputComponent->GetNumActionBindings(); i++)
		{
			const FInputActionBinding& Binding = InputComponent->GetActionBinding(i);
			if(Binding.GetActionName() == UInputBinding::GetEnumValueName<EnumType>(EnumValue) && Binding.KeyEvent == InputEvent && Binding.ActionDelegate.GetUObject() == UserObject)
			{
				InputComponent->RemoveActionBinding(i);
				break;
			}
		}
	}

	UFUNCTION(BlueprintCallable, Meta = (DisplayName = "Remove Input From Object", DefaultToSelf = "Pawn, UserObject", AutoCreateRefTerm = "InputName"), Category = "Input")
	static FORCEINLINE void RemoveInputUObject(class APawn* Pawn, class UObject* UserObject, const FName& InputName, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed)
	{
		if(!Pawn || !Pawn->Controller || !Pawn->Controller->InputComponent || !UserObject) return;
		UInputComponent* InputComponent = Pawn->Controller->InputComponent;
		for(int32 i = 0; i < InputComponent->GetNumActionBindings(); i++)
		{
			const FInputActionBinding& Binding = InputComponent->GetActionBinding(i);
			if(Binding.GetActionName() == InputName && Binding.KeyEvent == InputEvent && Binding.ActionDelegate.GetUObject() == UserObject)
			{
				InputComponent->RemoveActionBinding(i);
				break;
			}
		}
	}

	UFUNCTION(BlueprintCallable, Category = "Input")
	static FORCEINLINE void RemoveAllInputUObject(class APawn* Pawn, class UObject* UserObject)
	{
		if(!Pawn || !Pawn->Controller || !Pawn->Controller->InputComponent || !UserObject) return;
		UInputComponent* InputComponent = Pawn->Controller->InputComponent;
		for(int32 i = 0; i < InputComponent->GetNumActionBindings(); i++)
		{
			const FInputActionBinding& Binding = InputComponent->GetActionBinding(i);
			if(Binding.ActionDelegate.GetUObject() == UserObject)
			{
				InputComponent->RemoveActionBinding(i);
			}
		}
	}

	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "Pawn, UserObject", AutoCreateRefTerm = "FunctionName"), Category = "Input Binding")
	static FORCEINLINE void BindInput(class APawn* Pawn, class UObject* UserObject, const FName& FunctionName, const EInputBinding KeyBinding, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed)
	{
		BindInputUFunction(Pawn, UserObject, FunctionName, KeyBinding, InputEvent);
	}

	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "Pawn, UserObject"), Category = "Input Binding")
	static FORCEINLINE void RemoveInputFromObject(class APawn* Pawn, class UObject* UserObject, const EInputBinding KeyBinding, const TEnumAsByte<EInputEvent> InputEvent = IE_Pressed)
	{
		RemoveInputUObject(Pawn, UserObject, KeyBinding, InputEvent);
	}
};

USTRUCT(BlueprintType)
struct FInputBindingInfo
{
	GENERATED_BODY()
	
	FInputBindingInfo(){}
	FInputBindingInfo(const EInputBinding InputBind, const TEnumAsByte<EInputEvent> InputEvent, const FName& FuncName)
	{
		this->InputBind = InputBind; this->InputEvent = InputEvent; this->FuncName = FuncName;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Binding")
	EInputBinding InputBind = EInputBinding::PrimaryFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Binding")
	TEnumAsByte<EInputEvent> InputEvent = IE_Pressed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Binding")
	FName FuncName = NAME_None;
};

UCLASS(BlueprintType, Blueprintable)
class MULTIPLAYERSHOOTER_API UInputBindingHandler : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (DisplayName = "Binds"), Category = "Input Binding")
	TArray<FInputBindingInfo> BP_Binds;
	
	TArray<void*> ActiveCPP_Binds;

	template<typename UserClass, typename ReturnType, typename... ParamTypes, typename EnumType>
	void AddBindUObject(class APawn* Pawn, UserClass* UserObject, ReturnType(UserClass::* MemFuncPtr)(ParamTypes...), const EnumType EnumValue);

	template<typename Func, typename ... VarTypes, typename EnumType>
	void AddBindLambda(class APawn* Pawn, class UObject* UserObject, const Func&& FuncPtr, const EnumType EnumValue, VarTypes... Vars);
};


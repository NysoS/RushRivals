// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum_SkinPart.h"
#include "W_SkinPart.generated.h"

class UW_SkinPartSelector;
class UButton;
class UImage;

/**
 * 
 */
UCLASS()
class HB_API UW_SkinPart : public UUserWidget
{
	GENERATED_BODY()

friend UW_SkinPartSelector;

public:

	UFUNCTION(BlueprintCallable)
	void ApplyInfo(const FDataSkin& info, const FName& rowName);

protected:

	// Begin Widget
	virtual void NativeOnInitialized() override;
	// End Widget

private:

	UFUNCTION()
	void OnButtonPressed();

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> B_Default;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> I_Default;

private:

	FName m_RowName;

	UPROPERTY()
	FDataSkin CurrentInfo;

	TObjectPtr<UW_SkinPartSelector> ParentPtr;
	
};

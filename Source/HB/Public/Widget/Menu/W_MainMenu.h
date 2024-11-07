// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class HB_API UW_MainMenu : public UUserWidget
{
	GENERATED_BODY()

protected:

	// Begin UserWidget
	virtual void NativeOnInitialized() override;
	virtual bool NativeSupportsCustomNavigation() const override { return true; }
	// End UserWidget

	UFUNCTION()
	void GoLeftFunction();

	UFUNCTION(BlueprintNativeEvent)
	void K2_OnGoLeft();

protected:

	UPROPERTY()
	FOnInputAction OnGoLeft;
	
};

// Copyright Pinguin studio
#pragma once

#include "Blueprint/UserWidget.h"
#include "WidgetKeyBind.generated.h"

UCLASS(BlueprintType)
class HB_API UWidgetKeyBind : public UUserWidget
{
	GENERATED_BODY()

private:

	void Refresh();

public:

	// TODO mettre un argument à l'intérieur.
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInputTargetKey();

public:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> T_KeyDisplayName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UInputKeySelector> IKS_KeyAction;

protected:

	TPair<FName, FKey> TargetKey;

};
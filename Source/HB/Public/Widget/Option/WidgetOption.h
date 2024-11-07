// Copyright Pinguin studio
#pragma once

#include "Blueprint/UserWidget.h"
#include "WidgetOption.generated.h"

UCLASS(BlueprintType)
class HB_API UWidgetOption : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void OnOptionPressed();

private:



public:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> B_Option;

protected:

	TArray<TObjectPtr<UUserWidget>> CategoryListOption;
	
};
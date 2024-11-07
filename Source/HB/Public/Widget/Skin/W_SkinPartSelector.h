// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Enum_SkinPart.h"

#include "W_SkinPartSelector.generated.h"

enum ESkinPart;
class UDataTable;
class UScrollBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkinSelect, const FString&, key, const FName&, RowName);

/**
 * 
 */
UCLASS()
class HB_API UW_SkinPartSelector : public UUserWidget
{
	GENERATED_BODY()

public:

	// Begin - UUserWidget
	virtual void NativeOnInitialized() override;
	// End - UUserWidget
	
	UFUNCTION(BlueprintCallable)
	void ChangeData(const ESkinPart& key);

	void ApplySkin(const FName& data);

protected:

	UDataTable* GetDataTable(const FString& key) const;

public:

	UPROPERTY(BlueprintAssignable)
	FOnSkinSelect OnSkinSelect;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Property")
	TSubclassOf<UUserWidget> SkinPartClass;

	UPROPERTY(EditDefaultsOnly, Category = "Property")
	FMargin BoxMargin = {15.f, 15.f, 15.f, 15.f};

	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 1, UIMax = 5, ClampMin = 1), Category = "Property")
	int NbrPerRow = 3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SB_SkinPart;

private:

	FString m_SelectedKey;

	UPROPERTY()
	TObjectPtr<UDataTable> SelectdSkinDataTable;
	
};

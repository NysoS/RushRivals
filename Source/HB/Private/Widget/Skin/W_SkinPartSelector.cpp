// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Skin/W_SkinPartSelector.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "GameMode/HBPlayerState.h"
#include "Widget/Skin/Enum_SkinPart.h"
#include "Widget/Skin/W_SkinPart.h"
#include "Character/HBCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogWidget, Log, All);

namespace
{
	FString GetTypeKey(const ESkinPart& skinPart)
	{
		switch (skinPart)
		{
		case Head:
			return SkinHead::name;
		case Back:
			return SkinBack::name;
		case Legs:
			return SkinLegs::name;
		case Torso:
			return SkinTorso::name;
		case Foots:
			return SkinFeets::name;
		case Gloves:
			return SkinHand::name;
		default:
			break;
		}
		return FString();
	}
}

void UW_SkinPartSelector::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (TObjectPtr<AHBPlayerState> PlayerState = GetOwningPlayerState<AHBPlayerState>())
	{
		OnSkinSelect.AddDynamic(PlayerState, &AHBPlayerState::OnSkinChange);
	}
}

void UW_SkinPartSelector::ChangeData(const ESkinPart& key)
{
	check(SkinPartClass)

	m_SelectedKey = GetTypeKey(key);
	if (m_SelectedKey.IsEmpty())
	{
		return;
	}
	SelectdSkinDataTable = GetDataTable(m_SelectedKey);

	if (!SelectdSkinDataTable)
	{
		UE_LOG(LogWidget, Warning, TEXT("Texte is empty wsh"))
		return;
	}

	SB_SkinPart->ClearChildren();

	TArray<FName> RowNames = SelectdSkinDataTable->GetRowNames();

	TObjectPtr<UHorizontalBox> CurrentBox;
	for (int32 i = 0; i < RowNames.Num(); ++i)
	{
		float row = i%NbrPerRow;
		if (row == 0 || !CurrentBox)
		{
			CurrentBox = NewObject<UHorizontalBox>(this, "");
			TObjectPtr<UScrollBoxSlot> CurrentBoxSlot = Cast<UScrollBoxSlot>(SB_SkinPart->AddChild(CurrentBox));
			CurrentBoxSlot->SetPadding(BoxMargin);
		}

		FDataSkin* CurrentRow = SelectdSkinDataTable->FindRow<FDataSkin>(RowNames[i], "");

		TObjectPtr<UW_SkinPart> CurrentSkinPart = CreateWidget<UW_SkinPart>(GetOwningPlayer(), SkinPartClass);
		CurrentSkinPart->ApplyInfo(*CurrentRow, RowNames[i]);
		CurrentSkinPart->ParentPtr = this;

		TObjectPtr<UHorizontalBoxSlot> CurrentSlot = CurrentBox->AddChildToHorizontalBox(CurrentSkinPart);
		CurrentSlot->SetHorizontalAlignment(HAlign_Center);
		CurrentSlot->SetVerticalAlignment(VAlign_Center);
		CurrentSlot->SetPadding(FMargin(10.f));
	}
}

void UW_SkinPartSelector::ApplySkin(const FName& data)
{
	OnSkinSelect.Broadcast(m_SelectedKey, data);
}

UDataTable* UW_SkinPartSelector::GetDataTable(const FString& key) const
{
	return GetOwningPlayerPawn<AHBCharacter>()->DataSkinTable[key];
}

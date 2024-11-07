// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Skin/W_SkinPart.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Widget/Skin/W_SkinPartSelector.h"

void UW_SkinPart::ApplyInfo(const FDataSkin& info, const FName& rowName)
{
	CurrentInfo = info;
	m_RowName = rowName;

	I_Default->SetBrushFromSoftTexture(CurrentInfo.SkinPic, false);

}

void UW_SkinPart::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	B_Default->OnPressed.AddDynamic(this, &UW_SkinPart::OnButtonPressed);
}

void UW_SkinPart::OnButtonPressed()
{
	if (ParentPtr)
	{
		ParentPtr->ApplySkin(m_RowName);
	}
}

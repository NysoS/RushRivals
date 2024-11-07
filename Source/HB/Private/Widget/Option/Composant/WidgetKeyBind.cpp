// Copyright Pinguin Studio
#pragma once

#include "Widget/Option/Composant/WidgetKeyBind.h"

#include "Components/TextBlock.h"

void UWidgetKeyBind::Refresh()
{
	T_KeyDisplayName->SetText(FText::FromName(TargetKey.Key));
}

void UWidgetKeyBind::SetInputTargetKey()
{
}

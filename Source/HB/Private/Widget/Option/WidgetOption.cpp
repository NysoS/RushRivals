// Copyright Pinguin studio
#pragma once

#include "Widget/Option/WidgetOption.h"

#include "Components/Button.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"


DEFINE_LOG_CATEGORY_STATIC(LogWidgetOption, Log, All)

void UWidgetOption::NativeOnInitialized()
{
	if (B_Option)
	{
		B_Option->OnClicked.AddDynamic(this, &UWidgetOption::OnOptionPressed);
	}
}

void UWidgetOption::OnOptionPressed()
{
	// TODO Get the list input so it can be display all over screennnnnnnnnnnnnnnnnnn
	UE_LOG(LogWidgetOption, Display, TEXT("Option Pressed"));

	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetOwningLocalPlayer());
	check(Subsystem);

	for (FEnhancedActionKeyMapping& keymap : Subsystem->GetAllPlayerMappableActionKeyMappings())
	{
		if(!keymap.IsPlayerMappable())
		{
			continue;
		}
	}
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Menu/W_MainMenu.h"
#include "Components/InputComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogMenuWidget, Log, All);

void UW_MainMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UE_LOG(LogMenuWidget, Warning, TEXT("je fais les bind d'input"))
	
	if (!InputComponent)
		InitializeInputComponent();

	InputComponent->BindAction(FName("MenuGoLeft"), EInputEvent::IE_Pressed, this, &UW_MainMenu::GoLeftFunction);

}

void UW_MainMenu::GoLeftFunction()
{
	UE_LOG(LogMenuWidget, Display, TEXT("Je vais bien sur les cotes"))
	K2_OnGoLeft();
}

void UW_MainMenu::K2_OnGoLeft_Implementation()
{
}

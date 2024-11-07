// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/StartingState.h"

#include "Core/GameMode/EOSGameMode.h"

UStartingState::UStartingState()
{
	bUseHandleTick = false;
}

void UStartingState::InitState()
{
	Super::InitState();
}

void UStartingState::HandleState()
{
	Super::HandleState();
}

void UStartingState::ExitState()
{
	Super::ExitState();
}

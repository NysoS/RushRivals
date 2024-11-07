// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/ProgressState.h"

#include "Core/GameState/EOSGameState.h"


UProgressState::UProgressState()
{
	bUseHandleTick = false;
}

void UProgressState::InitState()
{
	Super::InitState();
}

void UProgressState::HandleState()
{
	Super::HandleState();
}

void UProgressState::ExitState()
{
	Super::ExitState();
}

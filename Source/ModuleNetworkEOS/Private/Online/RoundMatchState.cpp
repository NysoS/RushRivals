// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/RoundMatchState.h"

#include "Core/GameState/EOSGameState.h"
#include "Online/MatchContext.h"

URoundMatchState::URoundMatchState()
{
	bUseHandleTick = false;
}

void URoundMatchState::InitState()
{
	Super::InitState();
}

void URoundMatchState::HandleState()
{
	Super::HandleState();
}

void URoundMatchState::ExitState()
{
	Super::ExitState();
}

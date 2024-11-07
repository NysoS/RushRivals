// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/WaitingPlayersState.h"
#include "ModuleNetworkEOSModules.h"
#include "Core/GameState/EOSGameState.h"

UWaitingPlayersState::UWaitingPlayersState()
{
	bUseHandleTick = true;
}

void UWaitingPlayersState::InitState()
{
	Super::InitState();
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Init State WaitingPlayersState"));
}

void UWaitingPlayersState::HandleState()
{
	Super::HandleState();
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Handle WaitingPlayersState"));
}

void UWaitingPlayersState::ExitState()
{
	Super::ExitState();
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Exit State WaitingPlayersState"));
}

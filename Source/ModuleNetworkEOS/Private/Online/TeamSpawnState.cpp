// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/TeamSpawnState.h"
#include "ModuleNetworkEOSModules.h"
#include "Core/GameState/EOSGameState.h"

UTeamSpawnState::UTeamSpawnState()
{
	bUseHandleTick = false;
}

void UTeamSpawnState::InitState()
{
	Super::InitState();

	Super::InitState();
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Init TeamSpawn State"));
	if (!m_GameState)
	{
		return;
	}

	if (m_GameState->GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("SET TIMER"));
		m_GameState->SetNewStateAfterTimer(5.f, EMatchState::Progress);
	}
}

void UTeamSpawnState::HandleState()
{
	Super::HandleState();
}

void UTeamSpawnState::ExitState()
{
	Super::ExitState();
}

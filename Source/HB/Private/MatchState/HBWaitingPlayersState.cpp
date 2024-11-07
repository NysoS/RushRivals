// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchState/HBWaitingPlayersState.h"

#include "Core/GameMode/EOSGameMode.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Online/MatchContext.h"

class UEOSSessionSubsystem;

UHBWaitingPlayersState::UHBWaitingPlayersState()
{
}

void UHBWaitingPlayersState::InitState()
{
	Super::InitState();
	
	if (m_GameState)
	{
		m_MatchContext->matchMetaData.currentMatchState = EMatchState::WaitingPlayers;
		m_GameState->SendMatchMetaData(m_MatchContext->matchMetaData);
	}

	UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetWorld()->GetGameInstance());
	if (!SessionSubsystem)
	{
		return;
	}

	SessionSubsystem->StartSession();
}

void UHBWaitingPlayersState::HandleState()
{
	Super::HandleState();
	if (!m_GameState)
	{
		return;
	}

	if (m_GameState->GetLocalRole() == ROLE_Authority)
	{
		m_GameState->SetNewStateAfterTimer(5.f, EMatchState::Start);
	}
}

void UHBWaitingPlayersState::ExitState()
{
	Super::ExitState();
	if (m_GameState)
	{
		m_GameState->SendMatchMetaData(m_MatchContext->matchMetaData);
	}
}

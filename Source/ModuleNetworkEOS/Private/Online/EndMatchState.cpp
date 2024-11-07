// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/EndMatchState.h"
#include "Core/GameState/EOSGameState.h"
#include "Online/MatchContext.h"
#include "Teams/EOSTeamInfo.h"
#include "Teams/EOSTeamSubsystem.h"

class UEOSTeamSubsystem;

UEndMatchState::UEndMatchState()
{
	bUseHandleTick = false;
}

void UEndMatchState::InitState()
{
	Super::InitState();
}

void UEndMatchState::HandleState()
{
	Super::HandleState();
}

void UEndMatchState::ExitState()
{
	Super::ExitState();

	UEOSTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UEOSTeamSubsystem>();
	if (!TeamSubsystem)
		return;

	for (auto& [index, TeamInfo] : TeamSubsystem->GetTeams())
	{
		TeamInfo->SetResultMatch(INDEX_NONE);
		TeamInfo->ResetTeamScore();
	}

	m_GameState->SendMatchMetaData(m_MatchContext->matchMetaData);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchState/HBMatchHandlingState.h"

#include "Core/GameMode/EOSGameMode.h"
#include "Core/GameMode/EOSGameRule.h"
#include "Core/GameState/EOSGameState.h"
#include "Online/MatchContext.h"

class AEOSGameMode;

UHBMatchHandlingState::UHBMatchHandlingState()
{
}

void UHBMatchHandlingState::InitState()
{
	Super::InitState();
    if (!m_GameState)
    {
        return;
    }

    m_MatchContext->matchMetaData.currentMatchState = EMatchState::Progress;
    m_GameState->SendMatchMetaData(m_MatchContext->matchMetaData);

    if (m_GameState->GetLocalRole() == ROLE_Authority)
    {
        if (const AEOSGameMode* GameMode = GetWorld()->GetAuthGameMode<AEOSGameMode>())
        {
            const FEOSGameRule GameRule = GameMode->GetGameRules();

			m_GameState->SetNewStateAfterTimer(GameRule.MaxRoundTime, EMatchState::EndRound);
        }
    }
}

void UHBMatchHandlingState::HandleState()
{
	Super::HandleState();
}

void UHBMatchHandlingState::ExitState()
{
	Super::ExitState();
}

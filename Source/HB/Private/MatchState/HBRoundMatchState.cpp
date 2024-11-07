// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchState/HBRoundMatchState.h"

#include "Core/Character/EOSCharacter.h"
#include "Core/GameState/EOSGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Online/MatchContext.h"
#include "Player/EOSPlayerSpawnManagerComponent.h"
#include "Teams/EOSTeamSubsystem.h"
#include "Teams/EOSTeamInfo.h"

UHBRoundMatchState::UHBRoundMatchState()
	: m_bEndGame(false)
{
}

void UHBRoundMatchState::InitState()
{
	Super::InitState();

	UE_LOG(LogTemp, Warning, TEXT("RoundState Init"))

	if (!m_GameState)
	{
		return;
	}
	m_bEndGame = false;

	if (m_GameState->GetLocalRole() == ROLE_Authority)
	{
		if (const AEOSGameMode* GameMode = GetWorld()->GetAuthGameMode<AEOSGameMode>())
		{
			const FEOSGameRule GameRule = GameMode->GetGameRules();

			UEOSTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UEOSTeamSubsystem>();
			if (TeamSubsystem)
			{
				for (auto& [id, teamInfo] : TeamSubsystem->GetTeams())
				{
					if (teamInfo->GetTeamScore() >= GameRule.MaxRound)
					{
						m_bEndGame = true;
						m_GameState->UpdateCurrentMatchState(EMatchState::End);
						return;
					}
				}
			}
		}
	}

	m_MatchContext->matchMetaData.currentMatchState = EMatchState::EndRound;
	m_GameState->SendMatchMetaData(m_MatchContext->matchMetaData);

	AEOSCharacter* Character = Cast<AEOSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!Character)
	{
		return;
	}
	Character->InputDisable(true);

	if (m_GameState->GetLocalRole() == ROLE_Authority)
	{
		UEOSPlayerSpawnManagerComponent* PlayerSpawnManagerComponent = m_GameState->GetPlayerSpawnManager();
		if (!PlayerSpawnManagerComponent)
		{
			return;
		}
		PlayerSpawnManagerComponent->ResetPlayerStart();

		if (const AEOSGameMode* GameMode = GetWorld()->GetAuthGameMode<AEOSGameMode>())
		{
			const FEOSGameRule GameRule = GameMode->GetGameRules();
			if (m_GameState->PlayerArray.Num() < GameRule.MinPlayerToStart)
			{
				m_GameState->SetNewStateAfterTimer(GameRule.MaxTimeBeforeRestarted, EMatchState::End);
			}
			else
			{
				m_GameState->SetNewStateAfterTimer(GameRule.MaxTimeBeforeRestarted, EMatchState::Start);
			}
		}
	}
}

void UHBRoundMatchState::HandleState()
{
	Super::HandleState();
}

void UHBRoundMatchState::ExitState()
{
	Super::ExitState();

	UE_LOG(LogTemp, Warning, TEXT("RoundState Exit"))

	if (m_bEndGame)
	{
		return;
	};

	if (const UEOSTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UEOSTeamSubsystem>())
	{
		for (auto& [id, teamInfo] : TeamSubsystem->GetTeams())
		{
			teamInfo->ResetMatchResult();
		}
	}

	m_GameState->SendMatchMetaData(m_MatchContext->matchMetaData);
}

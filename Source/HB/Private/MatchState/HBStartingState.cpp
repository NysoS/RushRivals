// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchState/HBStartingState.h"

#include "Core/GameMode/EOSGameMode.h"
#include "Core/GameMode/EOSGameRule.h"
#include "Kismet/GameplayStatics.h"
#include "Player/EOSPlayerSpawnManagerComponent.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "Character/HBCharacter.h"
#include "Online/MatchContext.h"

class AEOSGameMode;

UHBStartingState::UHBStartingState()
{
}

void UHBStartingState::InitState()
{
	Super::InitState();
	if (!m_GameState)
	{
		return;
	}

	m_MatchContext->matchMetaData.currentMatchState= EMatchState::Start;
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

		for (const auto& playerState : m_GameState->PlayerArray)
		{
			const AEOSPlayerState* PlayerState = Cast<AEOSPlayerState>(playerState);
			if (!playerState)
			{
				continue;
			}
			PlayerSpawnManagerComponent->MovePlayerToSpawn(PlayerState);
		}

		
		if (const AEOSGameMode* GameMode = GetWorld()->GetAuthGameMode<AEOSGameMode>())
		{
			const FEOSGameRule GameRule = GameMode->GetGameRules();

			if (m_GameState->PlayerArray.Num() < GameRule.MinPlayerToStart)
			{
				m_GameState->SetNewStateAfterTimer(GameRule.MaxTimeBeforeStarted, EMatchState::WaitingPlayers);
			}
			else
			{
				m_GameState->SetNewStateAfterTimer(GameRule.MaxTimeBeforeRestarted, EMatchState::Progress);
			}
		}
	}
}

void UHBStartingState::HandleState()
{
	Super::HandleState();
}

void UHBStartingState::ExitState()
{
	Super::ExitState();

	AEOSCharacter* Character = Cast<AEOSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!Character)
	{
		return;
	}
	Character->InputDisable();

	if (m_GameState)
	{
		m_GameState->SendMatchMetaData(m_MatchContext->matchMetaData);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchState/HBEndMatchState.h"

#include "Core/GameMode/EOSGameMode.h"
#include "Core/GameMode/EOSGameRule.h"
#include "Kismet/GameplayStatics.h"
#include "Core/GameState/EOSGameState.h"
#include "Character/HBCharacter.h"
#include "Core/Subsystem/EOSLobbySubsystem.h"
#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Online/MatchContext.h"
#include "Player/EOSPlayerSpawnManagerComponent.h"

class AEOSGameMode;

UHBEndMatchState::UHBEndMatchState()
{
}

void UHBEndMatchState::InitState()
{
	Super::InitState();

	UE_LOG(LogTemp, Warning, TEXT("EndState Init"))

	if (!m_GameState)
	{
		return;
	}

	m_MatchContext->matchMetaData.currentMatchState = EMatchState::End;
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

		UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetWorld()->GetGameInstance());
		if (!SessionSubsystem)
		{
			return;
		}

		AEOSGameMode* GameMode = GetWorld()->GetAuthGameMode<AEOSGameMode>();
		if (GameMode)
		{
			const FEOSGameRule GameRule = GameMode->GetGameRules();
			m_GameState->SetNewStateAfterTimer(GameRule.MaxTimeBeforeRestarted, EMatchState::WaitingPlayers);

			FTimerHandle ResetStatus;
			GetWorld()->GetTimerManager().SetTimer(ResetStatus, [=]()
			{
					SessionSubsystem->EndSession();
			}, 10, false);
		}
	}

	if (m_GameState->GetLocalRole() == ROLE_Authority) {
		return;
	}

	FTimerHandle BackToLobby;
	GetWorld()->GetTimerManager().SetTimer(BackToLobby, this, &UHBEndMatchState::KickAll, 4, false);
}

void UHBEndMatchState::HandleState()
{
	Super::HandleState();
}

void UHBEndMatchState::ExitState()
{
	Super::ExitState();

	UE_LOG(LogTemp, Warning, TEXT("EndState Exit"))
}

void UHBEndMatchState::KickAll()
{
	UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetWorld()->GetGameInstance());
	if (LobbySubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("BackToLobby"))
		LobbySubsystem->BackToLobby();
	}
}

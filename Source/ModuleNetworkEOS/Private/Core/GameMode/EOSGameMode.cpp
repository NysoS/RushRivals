// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameMode/EOSGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ModuleNetworkEOSModules.h"
#include "Core/GameInstance/EOSGameInstance.h"
#include "Core/GameMode/EOSMatchData.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Online/MatchContext.h"
#include "Server/ServerInstance.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Network/Types/Request.h"
#include "Teams/EOSTeamCreationComponent.h"

AEOSGameMode::AEOSGameMode(const FObjectInitializer& ObjectInitializer)
{
	bReplicates = false;
	bDelayedStart = false;
	bUseSeamlessTravel = true;

	if(const UEOSGameInstance* OwnerGameInstance = Cast<UEOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		FName KeyRule = OwnerGameInstance->m_ServerInstance->GetServerInstanceName();
		FEOSGameRule* GameRule = OwnerGameInstance->m_ServerInstance->GetRule(KeyRule);

		if(GameRule)
		{
			m_CurrentGameRule = *GameRule;
		}
	}
}

void AEOSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(m_bStartRound)
	{
		m_TimerMatchValue = GetTimerMatch();
		if (m_ServerMatchData)
			m_ServerMatchData->TimerMatch = m_TimerMatchValue;
	}
}

APlayerController* AEOSGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* PlayerControllerToLogin = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	WARNING_LOG(TEXT("Login Options %s"), *Options);
	Request request;
	request.parseParams(Options);

	if (PlayerControllerToLogin)
	{
		AEOSPlayerState* PlayerState = PlayerControllerToLogin->GetPlayerState<AEOSPlayerState>();
		if (PlayerState)
		{
			WARNING_LOG(TEXT("PS - %s"), *PlayerState->GetName())
			PlayerState->LobbyId = request.getValueParameter<FString>(UEOSSessionSubsystem::R_PARAMS_LOBBY_ID);
		}
	}

	return PlayerControllerToLogin;
}

void AEOSGameMode::PostLogin(APlayerController* NewPlayer)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("New player loged %s"), *NewPlayer->GetName());
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		if (GameSession)
		{
			if (GetNumPlayers() > m_CurrentGameRule.MaxPlayer)
			{
				for (auto& player : m_gameState->PlayerArray)
				{
					GameSession->KickPlayer(NewPlayer, FText::FromString("Server full"));
				}
			}
		}

		FUniqueNetIdRepl UniqueNetIdRepl;
		if (NewPlayer->IsLocalController())
		{
			if (const ULocalPlayer* LocalPlayer = NewPlayer->GetLocalPlayer())
			{
				UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
			}
			else
			{
				UniqueNetIdRepl = GetRemoteUniqueNetId(NewPlayer->Player);
			}
		}
		else
		{
			UniqueNetIdRepl = GetRemoteUniqueNetId(NewPlayer->Player);
		}

		const TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
		if(!UniqueNetId)
		{
			return;
		}

		const UEOSGameInstance* GameInstance = GetGameInstance<UEOSGameInstance>();
		if(!GameInstance)
			return;

		UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetGameInstance());
		if (!SessionSubsystem) {
			return;
		}

		if(const IOnlineSubsystem* Subsystem = Online::GetSubsystem(NewPlayer->GetWorld()))
		{
			if(const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
			{
				FSessionInfo SessionInfo = SessionSubsystem->GetSessionInfo();

				if (SessionPtr->RegisterPlayer(SessionInfo.SessionName, *UniqueNetId, false))
				{
					UE_LOG(LogTemp, Warning, TEXT("Registration success"));
					if (AEOSGameState* MyGameState = GetWorld()->GetGameState<AEOSGameState>())
					{
						if(CanStartingMatch())
						{
							StartMatch();
						}

						APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>();
						if(NewPlayerState)
						{
							MyGameState->OnPlayerJoiningOrLeaving(NewPlayerState->GetPlayerName(), false);
						}

						if (AEOSPlayerState* EPS = Cast<AEOSPlayerState>(NewPlayerState))
						{
							UE_LOG(ModuleNetworkEOS, Warning, TEXT("Lobby ID %s"), *EPS->LobbyId)
						}

						UE_LOG(ModuleNetworkEOS, Warning, TEXT("Number players registered %i"), MyGameState->PlayerArray.Num());
					}


					SessionPtr->UpdateSession(SessionInfo.SessionName, *SessionPtr->GetSessionSettings(SessionInfo.SessionName), true);
				}
			}
		}
	}
}

void AEOSGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	if (AEOSGameState* MyGameState = GetWorld()->GetGameState<AEOSGameState>())
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Logout Number players registered %i"), MyGameState->PlayerArray.Num());
		for(auto& player : GameState->PlayerArray)
		{
			if(Exiting->GetPlayerState<APlayerState>() != player)
			{
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("player state id %d"), player->GetPlayerId());
				continue;
			}

			
			const TSharedPtr<const FUniqueNetId> UniqueNetId = GetUniqueNetId(player);
			if (!UniqueNetId)
			{
				UE_LOG(ModuleNetworkEOS, Error, TEXT("Logout : UniqueNetId is nullprt"));
				return;
			}

			UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetGameInstance());
			if (!SessionSubsystem) {
				return;
			}

			if(const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
			{
				if(const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
				{
					FSessionInfo SessionInfo = SessionSubsystem->GetSessionInfo();

					if (SessionPtr->UnregisterPlayer(SessionInfo.SessionName, *UniqueNetId))
					{
						SessionPtr->UpdateSession(SessionInfo.SessionName, *SessionPtr->GetSessionSettings(SessionInfo.SessionName), true);
						SessionPtr->ClearOnSessionParticipantRemovedDelegates(this);
						UE_LOG(LogTemp, Warning, TEXT("UnRegistration success"));

						AEOSPlayerController* ExitinPlayerController = Cast<AEOSPlayerController>(Exiting);
						if(!ExitinPlayerController)
						{
							return;
						}

						if (GetLocalRole() == ROLE_SimulatedProxy)
						{
							MyGameState->OnRep_NetMulticastRemovePlayeState(player->GetPlayerId());
						}
						MyGameState->OnPlayerJoiningOrLeaving(player->GetPlayerName(), true);
					}
					
					UE_LOG(ModuleNetworkEOS, Warning, TEXT("New Number players registered %i"), MyGameState->PlayerArray.Num());
				}
			}
		}
	}
}

void AEOSGameMode::BeginDestroy()
{
	Super::BeginDestroy();
}

void AEOSGameMode::UnRegisterPlayer(FName SessionName, const FUniqueNetId& UniqueNetId)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			if (SessionPtr->UnregisterPlayer(FName("Dev_Session"), UniqueNetId))
			{
				SessionPtr->UpdateSession(SessionName, *SessionPtr->GetSessionSettings(SessionName), true);
				SessionPtr->ClearOnSessionParticipantRemovedDelegates(this);
				UE_LOG(LogTemp, Warning, TEXT("UnRegistration success"));
			}
		}
	}
}

void AEOSGameMode::Destroyed()
{
	Super::Destroyed();
}

void AEOSGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void AEOSGameMode::StartRound()
{

	//GetWorld()->GetTimerManager().SetTimer(m_TimerMatchHandle, this, &AGameMode::EndMatch, m_CurrentGameRule.MaxRoundTime, false);
}

void AEOSGameMode::Travel(const FString& mapName, bool bAbsolute, bool bShouldSkipGameNotify)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("MapName %s"), *mapName);
	GetWorld()->GetTimerManager().ClearTimer(m_TimerMatchHandle);
}

float AEOSGameMode::GetTimerMatch() const
{
	return GetWorld()->GetTimerManager().GetTimerRemaining(m_TimerMatchHandle);
}

void AEOSGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEOSGameMode, m_TimerMatchValue);
}

AActor* AEOSGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AEOSGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if(!m_gameState)
	{
		return;
	}

	AEOSPlayerController* EOSNewPlayerController = Cast<AEOSPlayerController>(NewPlayer);
	if (!EOSNewPlayerController)
	{
		return;
	}

	UEOSTeamCreationComponent* TeamCreationComponent = m_gameState->GetComponentByClass<UEOSTeamCreationComponent>();
	if (!TeamCreationComponent)
	{
		return;
	}
	TeamCreationComponent->AffectationTeam(EOSNewPlayerController);
	TeamCreationComponent->OnRep_NetMulticastTeamPopulate();
}

const FEOSGameRule AEOSGameMode::GetGameRules() const
{
	return m_CurrentGameRule;
}

void AEOSGameMode::UpdateSessionState(const FMatchMetaData& MatchMetaData)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		return;
	}

	const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface();
	if (!SessionPtr)
	{
		return;
	}

	const UEOSGameInstance* GameInstance = GetGameInstance<UEOSGameInstance>();
	if (!GameInstance)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("GameInstance null"));
		return;
	}
	

	if (MatchMetaData.currentMatchState == EMatchState::Start)
	{
		SessionPtr->StartSession(GameInstance->NetworkData.SessionName);
		return;
	}

	if (MatchMetaData.currentMatchState == EMatchState::End)
	{
		SessionPtr->EndSession(GameInstance->NetworkData.SessionName);
	}
}
//
//void AEOSGameMode::OnRep_ServerCopyPlayerState_Implementation(const FPlayerStateCopyProperties& PlayerStateProperties)
//{
//	for(APlayerState* PlayerState : m_gameState->PlayerArray)
//	{
//		AEOSPlayerState* EOSPlayerState = Cast<AEOSPlayerState>(PlayerState);
//		if (PlayerState->GetPlayerId() == PlayerStateProperties.PlayerId && EOSPlayerState)
//		{
//			
//		}
//	}
//}

void AEOSGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AEOSGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AEOSGameState* EOSGameState = Cast<AEOSGameState>(GetWorld()->GetGameState()))
	{
		m_gameState = EOSGameState;

		m_gameState->OnUpdateMatchMetaData.AddUniqueDynamic(this, &AEOSGameMode::UpdateSessionState);
	}
}

bool AEOSGameMode::CanStartingMatch()
{
	if(!m_gameState)
	{
		return false;
	}

	if(m_gameState->PlayerArray.Num() >= m_CurrentGameRule.MinPlayerToStart)
	{
		if (m_gameState->GetMatchContext() && m_gameState->GetCurrentMatchState() == EMatchState::WaitingPlayers)
		{
			m_gameState->GetMatchContext()->ExecuteState();
		}
		return true;
	}

	m_gameState->UpdateCurrentMatchState(EMatchState::WaitingPlayers);
	return false;
}

FUniqueNetIdRepl AEOSGameMode::GetRemoteUniqueNetId(const TObjectPtr<UPlayer> PLayer) const
{
	if(const UNetConnection* RemoteNetConnectionRef = Cast<UNetConnection>(PLayer))
	{
		FUniqueNetIdRepl UniqueNetIdRepl = RemoteNetConnectionRef->PlayerId;
		return UniqueNetIdRepl.GetUniqueNetId();
	}

	return nullptr;
}

TSharedPtr<const FUniqueNetId> AEOSGameMode::GetUniqueNetId(const APlayerState* PlayerState) const
{
	FUniqueNetIdRepl UniqueNetIdRepl = PlayerState->GetUniqueId();
	return UniqueNetIdRepl.GetUniqueNetId();
}
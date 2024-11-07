// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameState/EOSGameState.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "ModuleNetworkEOS/Public/Online/MatchContext.h"
#include "Core/GameState/EOSTeamComponent.h"
#include "Core/Library/NetworkEOSBPFunctionLibrary.h"
#include "Core/Subsystem/EOSLobbySubsystem.h"
#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Network/Types/Request.h"
#include "Online/EndMatchState.h"
#include "Online/ProgressState.h"
#include "Online/StartingState.h"
#include "Online/WaitingPlayersState.h"
#include "Online/Session/LobbyInfo.h"
#include "Player/EOSPlayerSpawnManagerComponent.h"
#include "Teams/EOSTeamCreationComponent.h"

AEOSGameState::AEOSGameState()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	bAlwaysRelevant = true;
	NetPriority = 3.f;

	m_CurrentMatchState = EMatchState::NoState;

	m_MatchStatesProperties = {
		{ EMatchState::WaitingPlayers, UWaitingPlayersState::StaticClass()},
		{ EMatchState::Start, UStartingState::StaticClass()},
		{ EMatchState::Progress, UProgressState::StaticClass()},
		{ EMatchState::End, UEndMatchState::StaticClass()},
	};

	UEOSTeamComponent* TeamComponent = CreateDefaultSubobject<UEOSTeamComponent>(TEXT("TeamsComponent"));
	if(TeamComponent)
	{
		TeamComponent->SetIsReplicated(true);
		m_TeamComponent = TeamComponent;
	}

	m_PlayerSpawnManager = CreateDefaultSubobject<UEOSPlayerSpawnManagerComponent>(TEXT("EOS_PlayerSpawningComponent"));
	m_TeamCreationComponent = CreateDefaultSubobject<UEOSTeamCreationComponent>(TEXT("EOS_TeamCreationComponent"));

	CreateTeam();
}

void AEOSGameState::UpdateCurrentMatchState(const TEnumAsByte<EMatchState>& currentMatchState)
{
	m_CurrentMatchState = currentMatchState;
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("UpdateCurrentMatchState"));
	if(GetLocalRole() == ROLE_Authority)
	{
		OnNotify_UpdateState();
	}
}

void AEOSGameState::OnPlayerJoiningOrLeaving(const FString& newPlayerName, bool bIsLeave)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("OnNewPlayerJoining"));

	if (GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Info state to Server : new Player name joining %s"), *newPlayerName);
		OnRep_ClientInfoState(newPlayerName, bIsLeave);
	}
}

void AEOSGameState::OnRep_ClientInfoState_Implementation(const FString& newPlayerName, bool bIsLeave)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Info Client new Player has join"));
	OnPlayerJoinOrLeaveSession.Broadcast(newPlayerName, bIsLeave);
}

UMatchState* AEOSGameState::GetStateByType(const TEnumAsByte<EMatchState>& stateType) const
{
	if (m_MatchStates.Num() <= 0)
	{
		return nullptr;
	}

	return  *m_MatchStates.Find(stateType);
}

EMatchState AEOSGameState::GetCurrentMatchState() const
{
	return m_CurrentMatchState;
}

UMatchContext* AEOSGameState::GetMatchContext()
{
	if(!m_MatchContext)
	{
		return nullptr;
	}

	return m_MatchContext;
}

void AEOSGameState::TeamAffectationAfterNewPlayerJoin(AEOSPlayerState* NewPlayer)
{
	if (GetCurrentMatchState() != EMatchState::Start)
	{
		return;
	}
}

void AEOSGameState::CreateTeam() const
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Player count %d"), PlayerArray.Num());

	if(!m_TeamComponent)
	{
		return;
	}

	m_TeamComponent->CreateTeam();
}

void AEOSGameState::AffectationTeamAlreadyJoining()
{
	if (!m_TeamComponent)
	{
		return;
	}

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Affectation team"));

	m_TeamComponent->TeamAffectation();
}

void AEOSGameState::AffectationTeamJoiningAfterStartReady(AEOSPlayerState* NewPlayer)
{
#ifdef WITH_SERVER_CODE
	if (GetCurrentMatchState() == EMatchState::WaitingPlayers || GetCurrentMatchState() == EMatchState::Start)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Client : Affectation team for new player"));
		if (m_TeamComponent)
		{
			m_TeamComponent->TeamAddNewPlayer(NewPlayer);
		}
	}
#endif

	if (GetLocalRole() == ROLE_Authority)
	{
		if (UEOSTeamComponent* TeamComponent = GetTeamComponent())
		{
			TeamComponent->Test();
		}
	}
}

UEOSTeamComponent* AEOSGameState::GetTeamComponent() const
{
	return m_TeamComponent;
}

UEOSPlayerSpawnManagerComponent* AEOSGameState::GetPlayerSpawnManager() const
{
	return m_PlayerSpawnManager;
}


void AEOSGameState::SetTimer(float time)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Timer);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Timer, []() {}, time, false);
	}
}

void AEOSGameState::SetNewStateAfterTimer(float timer, const TEnumAsByte<EMatchState>& NewState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Timer);
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUFunction(this, "UpdateCurrentMatchState", NewState);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Timer, TimerDelegate, timer, false);
	}
}

float AEOSGameState::GetTimer() const
{
	return TimerValue;
}

void AEOSGameState::OnRemovePlayerState()
{
	UEOSTeamCreationComponent* TeamCreationComponent = GetComponentByClass<UEOSTeamCreationComponent>();
	if (!TeamCreationComponent)
	{
		return;
	}

	FTimerHandle TimerToRemovePlayerAtTeam;
	GetWorld()->GetTimerManager().SetTimer(TimerToRemovePlayerAtTeam, [=]()
		{
				TeamCreationComponent->OnRep_NetMulticastTeamPopulate();
		}, 3.f, false);
	
}

void AEOSGameState::SendMatchMetaData(FMatchMetaData& MetaData)
{
	if(&MetaData == nullptr)
		return;

	OnUpdateMatchMetaData.Broadcast(MetaData);
}

void AEOSGameState::OnRep_NetMutlicast_KickAll_Implementation()
{
	/*if (GetLocalRole() == ROLE_Authority)
	{
		return;
	}

	WARNING_LOG(TEXT("OnRep_NetMulticast_KickAll"))
	UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetGameInstance());
	if (!SessionSubsystem)
	{
		return;
	}

	FSessionInfo SessionInfo;
	if (CacheSubsystem->Get<FSessionInfo>(FString(UEOSSessionSubsystem::C_SESSION_INFO), SessionInfo))
	{
		if (!SessionInfo.JoinByFriendFromLobby)
		{
			SessionSubsystem->DestroySession(SessionInfo.SessionName);
		}
		else
		{
			UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetGameInstance());
			if (LobbySubsystem)
			{
				LobbySubsystem->BackToLobby();
			}
		}
	}*/
}

void AEOSGameState::BeginPlay()
{
	Super::BeginPlay();

	CacheSubsystem = UGameInstance::GetSubsystem<UCacheSubsystem>(GetGameInstance());
	if (!CacheSubsystem)
	{
		ERROR_LOG(TEXT("Impossible to get CacheSusystem"));
	}

	//if (CacheSubsystem)
	//{
	//	UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetGameInstance());
	//	if (!LobbySubsystem)
	//	{
	//		return;
	//	}

	//	FLobbyInfo LobbyInfo;
	//	CacheSubsystem->Get<FLobbyInfo>(UEOSLobbySubsystem::CACHE_LOBBY_INFO, LobbyInfo);
	//	if (!LobbyInfo.bIsOwner && !LobbyInfo.SessionName.ToString().IsEmpty())
	//	{
	//		LobbySubsystem->DestroyLobby(LobbyInfo.SessionName);
	//	}
	//}

	InitMatchContext();
}

void AEOSGameState::PostNetInit()
{
	Super::PostNetInit();
}

void AEOSGameState::BeginReplication()
{
	Super::BeginReplication();
}

void AEOSGameState::PostNetReceive()
{
	Super::PostNetReceive();
}

void AEOSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEOSGameState, TimerValue);
	DOREPLIFETIME(AEOSGameState, m_CurrentMatchState);
}

void AEOSGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(GetLocalRole() == ROLE_Authority)
	{
		TimerValue = GetWorld()->GetTimerManager().GetTimerRemaining(TimerHandle_Timer);
	}
}

void AEOSGameState::InitMatchContext()
{
	UMatchContext* MatchContext = NewObject<UMatchContext>(GetWorld());
	if (!MatchContext)
	{
		return;
	}
	m_MatchContext = MatchContext;

	for (auto [index, stateClass] : m_MatchStatesProperties)
	{
		m_MatchStates.Add(index, NewObject<UMatchState>(GetWorld(), stateClass));
	}

	if(GetLocalRole() == ROLE_Authority)
	{
		CreateTeam();
		m_MatchContext->SetDefaultState(GetStateByType(EMatchState::WaitingPlayers));
	}
}

void AEOSGameState::OnRep_NetMulticastRemovePlayeState_Implementation(const int32& PlayerStateId)
{
	OnRemovePlayerState();
}

void AEOSGameState::OnNotify_UpdateState()
{
	if(m_MatchContext)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Timer);
		if(GetStateByType(m_CurrentMatchState))
		{
			m_MatchContext->TransitionTo(GetStateByType(m_CurrentMatchState));
		}
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameState/EOSLobbyGameState.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/Subsystem/EOSLobbySubsystem.h"
#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AEOSLobbyGameState::AEOSLobbyGameState()
	: bIsLobbyActive(false), LobbyId(FString(""))
{
	bReplicates = true;
}

void AEOSLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, LobbyId);
}

void AEOSLobbyGameState::BeginPlay()
{
	Super::BeginPlay();

	m_CacheSubsytem = UGameInstance::GetSubsystem<UCacheSubsystem>(GetGameInstance());
	if (!m_CacheSubsytem)
	{
		ERROR_LOG(TEXT("Cache Subsytem on EOS LobbyGameState can't init"));
	}
}

void AEOSLobbyGameState::SetLoobyId(const FString& Id)
{
	LobbyId = Id;
}

FString AEOSLobbyGameState::GetLobbyId() const
{
	return LobbyId;
}

void AEOSLobbyGameState::OnRep_NetMulticast_FindMatchMakingWithLobby_Implementation()
{
	//UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetGameInstance());
	//if (!SessionSubsystem)
	//{
	//	return;
	//}

	//SessionSubsystem->OnMatchMakingProgressDelegate.Broadcast();
	//WARNING_LOG(TEXT("OnRep_FindMatchMakingWithLobby"))
}

void AEOSLobbyGameState::OnRep_NetMutlicast_JoinMatchMakinWithLobby_Implementation(const FSessionInfo& SessionInfo)
{
	//UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetGameInstance());
	//if (!SessionSubsystem)
	//{
	//	return;
	//}
	//SessionSubsystem->OnMatchMakingJoiningDelegate.Broadcast();

	//if (GetLocalRole() == ROLE_Authority)
	//{
	//	FTimerHandle TimerHandle;
	//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [=]()->void
	//		{
	//			JoinMatchMakingWithLobby(SessionInfo);
	//		}, 2.0f, false);
	//}
	//else
	//{
	//	FSessionInfo SessionInfoRef = SessionInfo;
	////	SessionInfoRef.JoinByFriendFromLobby = true;
	//	JoinMatchMakingWithLobby(SessionInfoRef);
	//	//SessionSubsystem->JoinMatchMaking(SessionInfo.SessionName, SessionSearchResultInfo.SessionSearchResult);
	//}
}

void AEOSLobbyGameState::JoinMatchMakingWithLobby(const FSessionInfo& SessionInfo)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}

	if (m_CacheSubsytem)
	{
		m_CacheSubsytem->Set(FString(UEOSSessionSubsystem::C_SESSION_INFO), SessionInfo);
	}

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Travel player to address %s"), *SessionInfo.RequestDto.url);
	PlayerController->ClientTravel(SessionInfo.RequestDto.url, TRAVEL_Absolute);
}

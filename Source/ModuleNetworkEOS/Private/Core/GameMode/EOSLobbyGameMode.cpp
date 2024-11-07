// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameMode/EOSLobbyGameMode.h"

#include "ModuleNetworkEOSModules.h"
#include "OnlineSubsystemUtils.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"

FUniqueNetIdRepl AEOSLobbyGameMode::GetRemoteUniqueNetId(const TObjectPtr<UPlayer> PLayer) const
{
	if (const UNetConnection* RemoteNetConnectionRef = Cast<UNetConnection>(PLayer))
	{
		FUniqueNetIdRepl UniqueNetIdRepl = RemoteNetConnectionRef->PlayerId;
		return UniqueNetIdRepl.GetUniqueNetId();
	}

	return nullptr;
}

TSharedPtr<const FUniqueNetId> AEOSLobbyGameMode::GetUniqueNetId(const APlayerState* PlayerState) const
{
	FUniqueNetIdRepl UniqueNetIdRepl = PlayerState->GetUniqueId();
	return UniqueNetIdRepl.GetUniqueNetId();
}

AEOSLobbyGameMode::AEOSLobbyGameMode(const  FObjectInitializer& ObjectInitializer)
{
}

void AEOSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("New player loged on lobby %s"), *NewPlayer->GetName());

	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
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
		if (!UniqueNetId)
		{
			return;
		}


		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(NewPlayer->GetWorld()))
		{
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
			{
				if (SessionPtr->RegisterPlayer(FName("Dev_Lobby"), *UniqueNetId, false))
				{
					UE_LOG(LogTemp, Warning, TEXT("Registration success"));
					SessionPtr->UpdateSession(FName("Dev_Lobby"), *SessionPtr->GetSessionSettings(FName("Dev_Lobby")), true);
				}
			}
		}
	}
}

void AEOSLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (AEOSGameState* MyGameState = GetWorld()->GetGameState<AEOSGameState>())
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Logout Number players registered %i"), MyGameState->PlayerArray.Num());
		for (auto& player : GameState->PlayerArray)
		{
			if (Exiting->GetPlayerState<APlayerState>() != player)
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

			if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
			{
				if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
				{
					if (SessionPtr->UnregisterPlayer(FName("Dev_Lobby"), *UniqueNetId))
					{
						SessionPtr->UpdateSession(FName("Dev_Lobby"), *SessionPtr->GetSessionSettings(FName("Dev_Lobby")), true);
						SessionPtr->ClearOnSessionParticipantRemovedDelegates(this);
						UE_LOG(LogTemp, Warning, TEXT("UnRegistration success"));

						AEOSPlayerController* ExitinPlayerController = Cast<AEOSPlayerController>(Exiting);
						if (!ExitinPlayerController)
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

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PlayerState/EOSPlayerState.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/GameInstance/EOSGameInstance.h"
#include "Net/UnrealNetwork.h"

AEOSPlayerState::AEOSPlayerState()
	: LobbyId(FString("")), bIsLobbyOwner(false)
{
	bReplicates = true;
	bActorSeamlessTraveled = true;
}

void AEOSPlayerState::BeginPlay()
{
	Super::BeginPlay();

	/*if (UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetGameInstance()))
	{
		if (GetLocalRole() != ROLE_Authority)
		{
			LobbyId = GameInstance->PlayerStateCopyPropertiesBetweenServer.LobbyId;
		}else
		{
			FPlayerStateCopyProperties customProperties;
			customProperties.PlayerId = GetPlayerId();
			customProperties.LobbyId = LobbyId;

			GameInstance->PlayerStateCopyPropertiesBetweenServer = customProperties;
		}
		
	}
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Go To Custom Copy properties"))*/
}

void AEOSPlayerState::BeginReplication()
{
	Super::BeginReplication();

	/*if (GetLocalRole() != ROLE_Authority)
	{
		UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetGameInstance());
		if (!GameInstance)
		{
			return;
		}

		CopyProperties(GameInstance->PlayerStateCopyPropertiesBetweenServer);
	}*/
}

void AEOSPlayerState::OnDeactivated()
{
	Super::OnDeactivated();
}

void AEOSPlayerState::OnRep_LobbyId()
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("OnRep_LobbyId"))
}

void AEOSPlayerState::SetLobbyId(const FString Id)
{
	LobbyId = Id;

	/*if (UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetGameInstance()))
	{
		GameInstance->PlayerStateCopyPropertiesBetweenServer.LobbyId = LobbyId;
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("GI LOBBY ID: %i"), GameInstance->PlayerStateCopyPropertiesBetweenServer.LobbyId);

	}*/

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("LOBBY ID: %s"), *LobbyId);
}

FString AEOSPlayerState::GetLobbyId() const
{
	return LobbyId;
}

void AEOSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, TeamIdN)
	DOREPLIFETIME(ThisClass, LobbyId)
}

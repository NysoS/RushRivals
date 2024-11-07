// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PlayerController/PlayerInfo.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

void UPlayerInfo::SetLobbyOwner(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	Owner = PlayerController;
	AddLobbyMember(PlayerController->GetPlayerState<APlayerState>());
}

bool UPlayerInfo::AddLobbyMember(TObjectPtr<APlayerState> PlayerState)
{
	if (MembersId.Contains(PlayerState))
	{
		return false;
	}

	MembersId.Add(PlayerState);
	return true;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Library/NetworkEOSBPFunctionLibrary.h"

#if WITH_EDITOR
#include "Editor.h"
#endif 

#include "Core/GameInstance/EOSGameInstance.h"
#include "Core/GameMode/EOSGameMode.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "ModuleNetworkEOSModules.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UWorld* UNetworkEOSBPFunctionLibrary::GetActiveWorld()
{
	UWorld* world = nullptr;
#if WITH_EDITOR
	if (GIsEditor)
	{
		if (GPlayInEditorID == -1)
		{
			FWorldContext* worldContext = GEditor->GetPIEWorldContext(1);
			if (worldContext == nullptr)
			{
				if (UGameViewportClient* viewport = GEngine->GameViewport)
				{
					world = viewport->GetWorld();
				}
			}
			else
			{
				world = worldContext->World();
			}
		}
		else
		{
			FWorldContext* worldContext = GEditor->GetPIEWorldContext(GPlayInEditorID);
			if (worldContext == nullptr)
			{
				return nullptr;
			}
			world = worldContext->World();
		}
	}
	else
	{
		world = GEngine->GetCurrentPlayWorld(nullptr);
	}
#else

	auto WorldContext = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
	if (!WorldContext)
	{
		return nullptr;
	}

	return WorldContext->World();

#endif

	return world;
}

UEOSGameInstance* UNetworkEOSBPFunctionLibrary::GetEOSGameInstance()
{
	UEOSGameInstance* gameInstance = Cast<UEOSGameInstance, UGameInstance>(GetActiveWorld()->GetGameInstance());
	if (!gameInstance)
	{
		return nullptr;
	}

	return gameInstance;
}

ACharacter* UNetworkEOSBPFunctionLibrary::GetCurrentPlayerCharacter()
{
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(GetActiveWorld(), 0);
	if (!Character)
		return nullptr;

	return Character;
}

AEOSPlayerController* UNetworkEOSBPFunctionLibrary::GetCurrentPlayerController()
{
	AEOSPlayerController* PlayerController = Cast<AEOSPlayerController>(UGameplayStatics::GetPlayerController(GetActiveWorld(),0));
	if (!PlayerController)
		return nullptr;

	return PlayerController;
}

AEOSPlayerController* UNetworkEOSBPFunctionLibrary::GetPlayerController(const UObject* WorldContextObject,
	int32 PlayerIndex)
{
	AEOSPlayerController* PlayerController = Cast<AEOSPlayerController>(UGameplayStatics::GetPlayerController(WorldContextObject, 0));
	if (!PlayerController)
		return nullptr;

	return PlayerController;
}

AEOSGameMode* UNetworkEOSBPFunctionLibrary::GetEOSGameGame()
{
	AEOSGameMode* GameMode = GetActiveWorld()->GetAuthGameMode<AEOSGameMode>();
	if (!GameMode)
		return nullptr;

	return GameMode;
}

AEOSGameState* UNetworkEOSBPFunctionLibrary::GetCurrentGameState()
{
	UWorld* World = GetActiveWorld();
	if (!World)
		return nullptr;

	return World->GetGameState<AEOSGameState>();
}

AEOSPlayerState* UNetworkEOSBPFunctionLibrary::GetCurrentPlayerState()
{
	const AEOSPlayerController* PlayerController = GetCurrentPlayerController();
	if (!PlayerController)
		return nullptr;

	return PlayerController->GetPlayerState<AEOSPlayerState>();
}

AEOSPlayerState* UNetworkEOSBPFunctionLibrary::GetPlayerStateByPlayerId(const int32& PLayerId)
{
	const AEOSGameState* GameState = GetCurrentGameState();
	if (!GameState) return nullptr;

	for(APlayerState* PLayerState : GameState->PlayerArray)
	{
		if (PLayerState->GetPlayerId() != PLayerId) continue;

		return Cast<AEOSPlayerState>(PLayerState);
	}

	return nullptr;
}

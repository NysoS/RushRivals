// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Network/Types/Request.h"
#include "EOSLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API AEOSLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

	FUniqueNetIdRepl GetRemoteUniqueNetId(const TObjectPtr<UPlayer> PLayer) const;
	TSharedPtr<const FUniqueNetId> GetUniqueNetId(const APlayerState* PlayerState) const;

public:
	AEOSLobbyGameMode(const FObjectInitializer& ObjectInitializer);

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};

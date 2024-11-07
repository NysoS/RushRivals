// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cache/CacheSubsystem.h"
#include "GameFramework/GameState.h"
#include "Online/Session/LobbyInfo.h"
#include "EOSLobbyGameState.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API AEOSLobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
	AEOSLobbyGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	UFUNCTION(Reliable, NetMulticast)
	void OnRep_NetMulticast_FindMatchMakingWithLobby();

	UFUNCTION(Reliable, NetMulticast)
	void OnRep_NetMutlicast_JoinMatchMakinWithLobby(const FSessionInfo& SessionInfo);

	UFUNCTION()
	void JoinMatchMakingWithLobby(const FSessionInfo& SessionInfo);

	UPROPERTY(EditAnywhere)
	bool bIsLobbyActive;

	void SetLoobyId(const FString& Id);

	UPROPERTY(Replicated, Getter=GetLobbyId)
	FString LobbyId;

	UFUNCTION(BlueprintGetter)
	FString GetLobbyId() const;

protected:

	UPROPERTY()
	UCacheSubsystem* m_CacheSubsytem;
};

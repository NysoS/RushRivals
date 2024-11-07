// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EOSGameRule.h"
#include "Core/NetworkSystem/NetworkData/NetworkData.h"
#include "GameFramework/GameMode.h"
#include "Online/MatchState.h"
#include "Online/MatchMetaData.h"
#include "EOSGameMode.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API AEOSGameMode : public AGameMode
{
	GENERATED_BODY()

	FUniqueNetIdRepl GetRemoteUniqueNetId(const TObjectPtr<UPlayer> PLayer) const;
	TSharedPtr<const FUniqueNetId> GetUniqueNetId(const APlayerState* PlayerState) const;

public:
	AEOSGameMode(const FObjectInitializer& ObjectInitializer);

	UFUNCTION()
	virtual void Tick(float DeltaSeconds) override;

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	UFUNCTION()
	virtual void PostLogin(APlayerController* NewPlayer) override;
	UFUNCTION()
	virtual void Logout(AController* Exiting) override;
	UFUNCTION()
	virtual void BeginDestroy() override;

	void UnRegisterPlayer(FName SessionName, const FUniqueNetId& UniqueNetId);

	UFUNCTION()
	virtual void Destroyed() override;
	UFUNCTION()
	virtual void HandleMatchHasStarted() override;

	UFUNCTION()
	void StartRound();

	UFUNCTION()
	void Travel(const FString& mapName, bool bAbsolute = false, bool bShouldSkipGameNotify = false);

	UFUNCTION(BlueprintCallable)
	float GetTimerMatch() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	const FEOSGameRule GetGameRules() const;

	UFUNCTION()
	void UpdateSessionState(const FMatchMetaData& MatchMetaData);

	/*UFUNCTION(Reliable, Server)
	void OnRep_ServerCopyPlayerState(const FPlayerStateCopyProperties& PlayerStateId);*/

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	UFUNCTION()
	virtual bool CanStartingMatch();
		 
	UPROPERTY()
	FEOSGameRule m_CurrentGameRule;

	UPROPERTY()
	FTimerHandle m_TimerMatchHandle;

	UPROPERTY(Replicated)
	float m_TimerMatchValue;

	UPROPERTY()
	bool m_bStartRound = false;

	UPROPERTY()
	class AEOSMatchData* m_ServerMatchData;

	UPROPERTY()
	AEOSGameState* m_gameState;

	UPROPERTY()
	TArray<APlayerController*> m_playerJoinedQueue;
};

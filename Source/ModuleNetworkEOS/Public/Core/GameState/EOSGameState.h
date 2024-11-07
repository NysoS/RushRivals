// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Online/MatchState.h"
#include "Online/TeamData.h"
#include "TimerManager.h"
#include "Cache/CacheSubsystem.h"
#include "Online/MatchMetaData.h"
#include "EOSGameState.generated.h"


/**
 * 
 */


class UEOSPlayerSpawnManagerComponent;
class UEOSTeamComponent;
class UEOSTeamCreationComponent;

UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API AEOSGameState : public AGameState
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerJoinOrLeaveSession, const FString&, PlayerName, bool, bIsLeave);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTeamsUpdate);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateMatchMetaData, const FMatchMetaData&, MatchMetaData);
	
public:
	AEOSGameState();

	UFUNCTION()
	virtual void UpdateCurrentMatchState(const TEnumAsByte<EMatchState>& currentMatchState);

	UFUNCTION()
	void OnPlayerJoiningOrLeaving(const FString& newPlayerName, bool bIsLeave);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_ClientInfoState(const FString& newPlayerName, bool bIsLeave);

	UFUNCTION()
	UMatchState* GetStateByType(const TEnumAsByte<EMatchState>& stateType) const;

	EMatchState GetCurrentMatchState() const;

	UFUNCTION()
	UMatchContext* GetMatchContext();

	UFUNCTION()
	virtual void OnNotify_UpdateState();

	UPROPERTY(BlueprintAssignable, Category = "EOS|Delegate|SessionNotify")
	FOnPlayerJoinOrLeaveSession OnPlayerJoinOrLeaveSession;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Delegate|MatchState")
	FOnTeamsUpdate OnTeamsUpdate;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Delegate|MatchState")
	FOnUpdateMatchMetaData OnUpdateMatchMetaData;

	UFUNCTION()
	virtual void TeamAffectationAfterNewPlayerJoin(AEOSPlayerState* NewPlayer);
	UFUNCTION()
	void CreateTeam() const;
	UFUNCTION()
	virtual void AffectationTeamAlreadyJoining();
	UFUNCTION()
	virtual void AffectationTeamJoiningAfterStartReady(AEOSPlayerState* NewPlayer);
	UFUNCTION(BlueprintCallable)
	UEOSTeamComponent* GetTeamComponent() const;
	UFUNCTION(BlueprintCallable)
	UEOSPlayerSpawnManagerComponent* GetPlayerSpawnManager() const;

	UFUNCTION()
	void SetTimer(float time);
	UFUNCTION()
	void SetNewStateAfterTimer(float timer, const TEnumAsByte<EMatchState>& NewState);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetTimer() const;

	UFUNCTION(NetMulticast, Client, Reliable)
	void OnRep_NetMulticastRemovePlayeState(const int32& PlayerStateId);
	UFUNCTION()
	void OnRemovePlayerState();

	UFUNCTION()
	void SendMatchMetaData(FMatchMetaData& MetaData);

	UFUNCTION(NetMulticast, Reliable)
	virtual void OnRep_NetMutlicast_KickAll();

protected:
	virtual void BeginPlay() override;
	virtual void PostNetInit() override;
	virtual void BeginReplication() override;
	virtual void PostNetReceive() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void InitMatchContext();

public:
	UPROPERTY(Replicated)
	float TimerValue;

	UPROPERTY()
	FTimerHandle TimerHandle_Timer;

protected:
	UPROPERTY(ReplicatedUsing = OnNotify_UpdateState)
	TEnumAsByte<EMatchState> m_CurrentMatchState = EMatchState::NoState;

	UPROPERTY()
	TObjectPtr<class UMatchContext> m_MatchContext;

	UPROPERTY(EditAnywhere, Category = "Match State Properties")
	TMap<TEnumAsByte<EMatchState>, TSubclassOf<UMatchState>> m_MatchStatesProperties;
	UPROPERTY()
	TMap<TEnumAsByte<EMatchState>, UMatchState*> m_MatchStates;

	UPROPERTY(EditAnywhere)
	UEOSTeamComponent* m_TeamComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UEOSTeamCreationComponent> m_TeamCreationComponent;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UEOSPlayerSpawnManagerComponent> m_PlayerSpawnManager;

	UPROPERTY()
	UCacheSubsystem* CacheSubsystem;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/NetworkSystem/NetworkData/NetworkData.h"
#include "GameFramework/PlayerState.h"
#include "EOSPlayerState.generated.h"


class UEOSTeamDisplayAsset;
class AEOSTeamInfo;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API AEOSPlayerState : public APlayerState
{
	GENERATED_BODY()


public:
	AEOSPlayerState();

	virtual void BeginPlay() override;
	virtual void BeginReplication() override;
	virtual void OnDeactivated() override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team Properties")
	int32 TeamIdN = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Team Properties")
	UEOSTeamDisplayAsset* TeamDisplayAsset = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyId, BlueprintGetter = GetLobbyId)
	FString LobbyId;

	UPROPERTY(Replicated)
	bool bIsLobbyOwner;

	UFUNCTION()
	void OnRep_LobbyId();

	UFUNCTION(BlueprintCallable)
	void SetLobbyId(const FString Id);
	UFUNCTION(BlueprintCallable, BlueprintGetter)
	FString GetLobbyId() const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfo.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API UPlayerInfo : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetLobbyOwner(APlayerController* PlayerController);

	bool AddLobbyMember(TObjectPtr<APlayerState> PlayerState);

protected:
	UPROPERTY()
	APlayerController* Owner;

	UPROPERTY()
	TArray<TObjectPtr<APlayerState>> MembersId;
};

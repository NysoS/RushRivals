// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/MatchState.h"
#include "WaitingPlayersState.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API UWaitingPlayersState : public UMatchState
{
	GENERATED_BODY()

public:
	UWaitingPlayersState();
	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;
};

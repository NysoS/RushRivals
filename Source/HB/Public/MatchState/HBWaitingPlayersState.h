// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/WaitingPlayersState.h"
#include "HBWaitingPlayersState.generated.h"

/**
 * 
 */
UCLASS()
class HB_API UHBWaitingPlayersState : public UWaitingPlayersState
{
	GENERATED_BODY()

public:
	UHBWaitingPlayersState();
	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;
};


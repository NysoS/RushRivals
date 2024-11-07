// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/MatchState.h"
#include "TeamSpawnState.generated.h"

/**
 * 
 */
UCLASS()
class MODULENETWORKEOS_API UTeamSpawnState : public UMatchState
{
	GENERATED_BODY()

public:
	UTeamSpawnState();

	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;
};

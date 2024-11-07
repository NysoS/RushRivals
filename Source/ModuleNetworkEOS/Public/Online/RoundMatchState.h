// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/MatchState.h"
#include "RoundMatchState.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API URoundMatchState : public UMatchState
{
	GENERATED_BODY()

public:
	URoundMatchState();

	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;
};

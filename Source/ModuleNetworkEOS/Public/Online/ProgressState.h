// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/MatchState.h"
#include "ProgressState.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API UProgressState : public UMatchState
{
	GENERATED_BODY()
	
public:
	UProgressState();

	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;
};

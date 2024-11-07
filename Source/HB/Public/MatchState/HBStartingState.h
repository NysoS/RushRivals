// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/StartingState.h"
#include "HBStartingState.generated.h"

/**
 * 
 */
UCLASS()
class HB_API UHBStartingState : public UStartingState
{
	GENERATED_BODY()

public:
	UHBStartingState();

	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;
};

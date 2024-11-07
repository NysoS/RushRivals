// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/ProgressState.h"
#include "HBMatchHandlingState.generated.h"

/**
 * 
 */
UCLASS()
class HB_API UHBMatchHandlingState : public UProgressState
{
	GENERATED_BODY()

public:
	UHBMatchHandlingState();

	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/RoundMatchState.h"
#include "HBRoundMatchState.generated.h"

/**
 * 
 */
UCLASS()
class HB_API UHBRoundMatchState : public URoundMatchState
{
	GENERATED_BODY()

public:
	UHBRoundMatchState();

	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;

private:
	UPROPERTY()
	bool m_bEndGame;
};


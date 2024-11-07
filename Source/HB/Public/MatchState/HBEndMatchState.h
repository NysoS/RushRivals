// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Online/EndMatchState.h"
#include "HBEndMatchState.generated.h"

/**
 * 
 */
UCLASS()
class HB_API UHBEndMatchState : public UEndMatchState
{
	GENERATED_BODY()

public:
	UHBEndMatchState();

	virtual void InitState() override;
	virtual void HandleState() override;
	virtual void ExitState() override;

	void KickAll();
};

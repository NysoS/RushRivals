// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateActions.h"
#include "MatchState.generated.h"

class UMatchContext;
class AEOSGameState;

UENUM(BlueprintType, Blueprintable)
enum EMatchResult
{
	Draw = 0,
	Victory,
	Defeat
};

/**
 * 
 */

UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API UMatchState : public UObject
{
	GENERATED_BODY()


public:
	UMatchState();
	void SetContext(UMatchContext* context);

	UFUNCTION()
	virtual void InitState();
	UFUNCTION()
	virtual void HandleState();
	UFUNCTION()
	virtual void ExitState();

	UFUNCTION()
	void SwitchState(const TEnumAsByte<EMatchState>& state) const;

	UPROPERTY()
	bool bUseHandleTick = true;

protected:
	UPROPERTY()
	UMatchContext* m_MatchContext = nullptr;
	UPROPERTY()
	AEOSGameState* m_GameState = nullptr;
};

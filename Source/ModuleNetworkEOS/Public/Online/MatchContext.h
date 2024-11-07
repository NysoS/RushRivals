// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MatchData.h"
#include "Core/GameMode/EOSGameMode.h"
#include "UObject/NoExportTypes.h"
#include "MatchContext.generated.h"

class AEOSGameState;
class UMatchState;

/**
 * 
 */
UCLASS()
class MODULENETWORKEOS_API UMatchContext : public UObject
{
	GENERATED_BODY()

public:
	UMatchContext();
	void SetDefaultState(UMatchState* defaultState);
	void TransitionTo(UMatchState* newState);

	void InitState() const;
	void ExecuteState() const;
	void ExitState() const;

	void UpdateState(const TEnumAsByte<EMatchState>& newState) const;

	UMatchState* GetCurrentState() const;
	AEOSGameState* GetGameState() const;

	UPROPERTY()
	FMatchMetaData matchMetaData;

protected:
	UPROPERTY()
	UMatchState* m_State;

	UPROPERTY()
	AEOSGameState* m_GameState;
};

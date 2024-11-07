// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MatchState.h"
#include "StateActions.h"
#include "MatchMetaData.generated.h"
/**
 * 
 */

UENUM(BlueprintType, Blueprintable)
enum EMatchStepState
{
	NoStep = 0,
	Init,
	Handling,
	Exit
};

USTRUCT(BlueprintType)
struct MODULENETWORKEOS_API FMatchMetaData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MetaData")
	TEnumAsByte<EMatchStepState> CycleStepState = EMatchStepState::NoStep;

	UPROPERTY(BlueprintReadOnly, Category = "MetaData")
	TEnumAsByte<EMatchState> currentMatchState = EMatchState::NoState;

	UPROPERTY(BlueprintReadOnly)
	FString data = "";
};

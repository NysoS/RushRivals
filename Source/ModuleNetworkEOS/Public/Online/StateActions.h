#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum EMatchState
{
	NoState = 0,
	WaitingPlayers,
	Start,
	Progress,
	EndRound,
	End
};
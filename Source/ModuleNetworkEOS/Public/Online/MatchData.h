#pragma once

#include "CoreMinimal.h"
#include "MatchData.generated.h"

USTRUCT(BlueprintType)
struct FMatchData
{
	GENERATED_BODY()

	UPROPERTY()
	FTimerHandle m_TimerMatchHandle;

};
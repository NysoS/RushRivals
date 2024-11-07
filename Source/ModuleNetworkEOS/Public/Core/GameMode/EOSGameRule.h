#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EOSGameRule.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct MODULENETWORKEOS_API FEOSGameRule : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayer = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPlayerToStart = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTimeBeforeStarted = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTimeBeforeRestarted = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRoundTime = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRoundToWin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTeamMember = 0;
};
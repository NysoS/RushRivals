#pragma once

#include "CoreMinimal.h"
#include "MatchState.h"
#include "UObject/ObjectMacros.h"
#include "TeamData.generated.h"

class AEOSPlayerState;

UENUM()
enum ETeamId
{
	NoTeam = 0,
	TeamA,
	TeamB
};

USTRUCT(BlueprintType, Blueprintable)
struct FTeamData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "EOS|Teams|Properties", BlueprintReadWrite)
	TArray<TObjectPtr<AEOSPlayerState>> members;

	UPROPERTY(EditAnywhere, Category = "EOS|Teams|Properties", BlueprintReadWrite)
	int32 score = 0;

	UPROPERTY(EditAnywhere, Category = "EOS|Teams|Properties", BlueprintReadWrite)
	int32 roundWin = 0;

	UPROPERTY(EditAnywhere, Category = "EOS|Teams|Properties", BlueprintReadWrite)
	FColor color = FColor::White;

	UPROPERTY(EditAnywhere, Category = "EOS|Teams|Properties", BlueprintReadWrite)
	FName name = "";

	UPROPERTY(EditAnywhere, Category = "EOS|Teams|Properties", BlueprintReadWrite)
	TEnumAsByte<ETeamId> teamId = ETeamId::NoTeam;

	UPROPERTY(EditAnywhere, Category = "EOS|Teams|Properties", BlueprintReadWrite)
	TArray<class APlayerStart*> spawnPoint;

	UPROPERTY(EditAnywhere, Category = "EOS|Teams|Properties", BlueprintReadWrite)
	TEnumAsByte<EMatchResult> teamResultEndMatch = EMatchResult::Draw;
};

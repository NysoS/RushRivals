#pragma once

#include "RequestDTO.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct MODULENETWORKEOS_API FRequestDTO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ip;

	UPROPERTY(BlueprintReadWrite)
	FString port;

	UPROPERTY(BlueprintReadWrite)
	FString params;

	UPROPERTY(BlueprintReadWrite)
	FString url;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "EOSPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class MODULENETWORKEOS_API AEOSPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	AEOSPlayerStart(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Team Property")
	int32 TeamId = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Properties|Spawn")
	bool bPlayerSpawned = false;
};

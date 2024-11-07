// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HBSaveGame.generated.h"

class AHBPlayerState;

/**
 * 
 */
UCLASS()
class HB_API UHBSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	
	UHBSaveGame();

	UPROPERTY()
	TMap<FString, FName> SelectedSkin;
	
};

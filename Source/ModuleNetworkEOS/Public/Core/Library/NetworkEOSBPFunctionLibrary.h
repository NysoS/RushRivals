// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NetworkEOSBPFunctionLibrary.generated.h"

class AEOSPlayerController;
class UModuleNetworkEOSSettings;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API UNetworkEOSBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static UWorld* GetActiveWorld();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static class UEOSGameInstance* GetEOSGameInstance();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static ACharacter* GetCurrentPlayerCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static AEOSPlayerController* GetCurrentPlayerController();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static AEOSPlayerController* GetPlayerController(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static class AEOSGameMode* GetEOSGameGame();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static class AEOSGameState* GetCurrentGameState();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static class AEOSPlayerState* GetCurrentPlayerState();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Library")
	static AEOSPlayerState* GetPlayerStateByPlayerId(const int32& PLayerId);
};

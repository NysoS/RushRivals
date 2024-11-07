// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/NetworkSystem/NetworkData/NetworkData.h"
#include "Engine/GameInstance.h"
#include "EOSGameInstance.generated.h"

#define LOG_ERROR(Txt) UE_LOG(LogTemp, Error, Txt)
#define LOG_WARNING(Txt) UE_LOG(LogTemp, Warning, Txt)

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API UEOSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Functions")
	FString GetPlayerUsername() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EOS Functions")
	bool GetPlayerStatus() const;

	UEOSGameInstance();
	virtual void Init() override;
	virtual void BeginDestroy() override;

	UFUNCTION()
	virtual void BeginLoadingScreen(const FString& MapName);
	UFUNCTION()
	virtual void EndLoadingScreen(UWorld* World);

	UFUNCTION()
	void OnConnectStatusChangeComplete(UWorld* World, bool bIsOnline);

	void HandleNetworkFailureDele(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	UFUNCTION()
	void AutoLogin();

	UPROPERTY()
	FNetworkData NetworkData;

	UPROPERTY()
	class UServerInstance* m_ServerInstance;

	UPROPERTY()
	FPlayerStateCopyProperties PlayerStateCopyPropertiesBetweenServer;
};

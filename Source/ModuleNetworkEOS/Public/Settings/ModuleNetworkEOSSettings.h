// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameMode/EOSGameRule.h"
#include "Engine/DeveloperSettings.h"
#include "ModuleNetworkEOSSettings.generated.h"

/**
 * 
 */
UCLASS(Config = EOS, DefaultConfig, meta = (DisplayName = "Network EOS Settings", BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class MODULENETWORKEOS_API UModuleNetworkEOSSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Testing")
	bool bActiveOnlineEOS = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Connect|DevAuth")
	bool bUseAccountPortal = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Connect|DevAuth", meta = (EditCondition = "!bUseAccountPortal"))
	FString IpSdk;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Connect|DevAuth", meta = (EditCondition = "!bUseAccountPortal"))
	FString PortSdk;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Connect|DevAuth|Users")
	TArray<FString> Users;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Connect|DevAuth|Users")
	FString DefaultUser;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Session|Default Map Session")
	bool bUseTransitionMap = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Session|Default Map Session")
	TSoftObjectPtr<UWorld> DefaultSessionMapText;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Session|Default Map Session", meta = (EditCondition="bUseTransitionMap"))
	TSoftObjectPtr<UWorld> DefaultTransitionMapText;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Session|Default Map Session")
	TSoftObjectPtr<UWorld> DefaultMenuMapText;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Server|GameMode")
	TSoftObjectPtr<UDataTable> GameRulesDataTable;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Server|GameMode", meta = (GetOptions = "GetGameModeInstanceType"))
	FName GameModeIntanceSelected;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Loading Screen")
	TSubclassOf<UUserWidget> LoadingScreenClass;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UFUNCTION()
	TArray<FName> GetGameModeInstanceType() const
	{
		TArray<FEOSGameRule*> rows;
		TArray<FName> values;

		if(!GameRulesDataTable.Get())
		{
			return values;
		}

		const FString context = "";
		values = GameRulesDataTable->GetRowNames();
		
		return values;
	}
};

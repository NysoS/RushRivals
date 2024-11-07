// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/NetworkSystem/NetworkData/NetworkData.h"
#include "Settings/ModuleNetworkEOSSettings.h"
#include "UObject/NoExportTypes.h"
#include "ServerInstance.generated.h"

USTRUCT(BlueprintType)
struct FServerInstanceProps
{
	GENERATED_BODY()

	UPROPERTY()
	FName InstanceName;

	EServerType ServerInstanceType;
};

class UGameInstance;

/**
 * 
 */
UCLASS()
class MODULENETWORKEOS_API UServerInstance : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void InitInstance(class UGameInstance* GameInstance);
	UFUNCTION()
	void SwitchMap();

	UFUNCTION()
	FName GetServerInstanceName() const;

	EServerType GetServerInstanceType() const;

	UFUNCTION()
	virtual void CreateInstance();

	FEOSGameRule* GetRule(FName& key) const;

protected:
	UPROPERTY()
	FServerInstanceProps m_ServerProps;
	UPROPERTY()
	class UGameInstance* m_GameInstance = nullptr;
	UPROPERTY()
	TObjectPtr<UModuleNetworkEOSSettings> m_NetworkSettings = nullptr;

};

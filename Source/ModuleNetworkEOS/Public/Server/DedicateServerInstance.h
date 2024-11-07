// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Server/ServerInstance.h"
#include "DedicateServerInstance.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API UDedicateServerInstance : public UServerInstance
{
	GENERATED_BODY()

public:
	virtual void InitInstance(UGameInstance* GameInstance) override;
	virtual void CreateInstance() override;
	virtual void ResetInstance();
	void RestartInstance();
};

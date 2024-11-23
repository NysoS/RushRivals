// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSGameInstanceSubsystem.generated.h"

class IOnlineSubsystem;

/**
 * 
 */
UCLASS()
class MODULENETWORKEOS_API UEOSGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	TUniquePtr<IOnlineSubsystem> EOSOnlineSubsytem;
};

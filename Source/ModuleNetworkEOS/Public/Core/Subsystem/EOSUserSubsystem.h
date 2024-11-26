// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystem/EOSGameInstanceSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "EOSUserSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MODULENETWORKEOS_API UEOSUserSubsystem : public UEOSGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "EOS|User|Functions")
	bool GetPlayerStatus() const;

	UFUNCTION(BlueprintCallable, Category = "EOS|User|Functions")
	FString GetPlayerUsername() const;

private:
	IOnlineIdentityPtr m_IdentityPtr;
};

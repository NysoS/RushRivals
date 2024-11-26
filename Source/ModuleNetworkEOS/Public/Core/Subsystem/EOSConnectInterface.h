// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "UObject/Interface.h"
#include "EOSConnectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEOSConnectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MODULENETWORKEOS_API IEOSConnectInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool Login() = 0;
	UFUNCTION()
	virtual bool LoginWithDevAuth(const FString& Id, const FString& Token) = 0;
	virtual void OnLoginProgress(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error) = 0;

	UFUNCTION()
	virtual void Logout(int32 LocalUserNum = 0) = 0;
	virtual void OnLogoutProgress(int32 LocalUserNum, bool bWasSuccessful) = 0;
};

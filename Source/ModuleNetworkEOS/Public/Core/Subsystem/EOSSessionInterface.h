// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "UObject/Interface.h"
#include "EOSSessionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEOSSessionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MODULENETWORKEOS_API IEOSSessionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION()
	virtual void CreateSession(bool bIsDedicateServer, int32 NumberOfPublicConnection) = 0;
	virtual void OnCreateSessionProgress(FName SessionName, bool bWasSuccessful) = 0;

	UFUNCTION()
	virtual void CreateSessionSever(int32 MaxPlayer = 0) = 0;
	virtual void OnCreateSessionServerProgress(FName SessionName, bool bWasSuccessful) = 0;

	UFUNCTION()
	virtual bool DestroySession(FName SessionName) = 0;
	virtual void OnDestroySessionProgress(FName SessionName, bool bWasSuccessful) = 0;
	UFUNCTION()
	virtual bool DestroySessionAllClient(FName SessionName) = 0;

	UFUNCTION()
	virtual bool FindSession() = 0;
	virtual void OnFindSessionCompleted(bool bWasSuccessful) = 0;

	UFUNCTION()
	virtual void JoinSession() = 0;
	virtual void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result) = 0;

	UFUNCTION()
	virtual bool FindMatchMaking() = 0;
	virtual void OnFindMatchMakingCompleted(bool bWasSuccessful) = 0;

	UFUNCTION()
	virtual void JoinMatchMaking() = 0;
	virtual void OnJoinMatchMakingCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result) = 0;
	UFUNCTION()
	virtual void MatchMaking() = 0;

	UFUNCTION()
	virtual void QuitSession(FName SessionName) = 0;
};

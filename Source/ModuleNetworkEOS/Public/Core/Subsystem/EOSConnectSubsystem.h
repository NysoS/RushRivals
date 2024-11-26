// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EOSConnectInterface.h"
#include "EOSGameInstanceSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "EOSConnectSubsystem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API UEOSConnectSubsystem : public UEOSGameInstanceSubsystem, public IEOSConnectInterface
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoginProgress);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginComplete, bool, bWasSuccessful);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLogoutComplete, int32, LocalUserNum, bool, bWasSuccessful);

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Connect|Functions")
	virtual bool Login() override;
	UFUNCTION(BlueprintCallable, Category = "EOS|Connect|Functions")
	virtual bool LoginWithDevAuth(const FString& Id, const FString& Token) override;
	virtual void OnLoginProgress(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error) override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Connect|Functions")
	bool LoginWithLuncher();
	void OnAutoLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	UFUNCTION(BlueprintCallable, Category = "EOS|Connect|Functions")
	virtual void Logout(int32 LocalUserNum = 0) override;
	virtual void OnLogoutProgress(int32 LocalUserNum, bool bWasSuccessful) override;

	UPROPERTY(BlueprintAssignable)
	FOnLoginProgress OnLoginProgressDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnLoginComplete OnLoginComplete;

	UPROPERTY(BlueprintAssignable)
	FOnLogoutComplete OnLogoutComplete;

private:
	IOnlineIdentityPtr m_IdentityPtr;
};

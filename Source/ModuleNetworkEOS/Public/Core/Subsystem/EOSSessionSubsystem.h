// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EOSSessionInterface.h"
#include "EOSGameInstanceSubsystem.h"
#include "Online/Session/LobbyInfo.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSSessionSubsystem.generated.h"

class UCacheSubsystem;

UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API UEOSSessionSubsystem : public UEOSGameInstanceSubsystem, public IEOSSessionInterface
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreateSessionComplete, FName, SessionName, bool, bWasSuccessful);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestroySessionComplete, FName, SessionName, bool, bWasSuccessful);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchMakingProgress);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchMakingJoining);

public:
	static constexpr const char* SESSION_SETTINGS_PORT = "PORT";
	static constexpr const char* SESSION_SETTINGS_SESSION_NAME = "SESSION_NAME";

	static constexpr const char* SESSION_SETTINGS_SHOW_ALL_SESSIONS = "DEV_SESSION";
	static constexpr const char* SESSION_SETTINGS_SHOW_ALL_SESSIONS_VALUE = "DEV";

	static constexpr const char* R_PARAMS_LOBBY_ID = "LOBBY_ID";

	static constexpr const char* C_SESSION_INFO = "CACHE_SESSION_INFO";

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Session|Functions")
	virtual void CreateSession(bool bIsDedicateServer, int32 NumberOfPublicConnection) override;
	virtual void OnCreateSessionProgress(FName SessionName, bool bWasSuccessful) override;

	UFUNCTION()
	virtual void CreateSessionSever(int32 MaxPlayer = 0) override;
	virtual void OnCreateSessionServerProgress(FName SessionName, bool bWasSuccessful) override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Session|Functions")
	virtual bool DestroySession(FName SessionName) override;
	virtual void OnDestroySessionProgress(FName SessionName, bool bWasSuccessful) override;
	UFUNCTION()
	virtual bool DestroySessionAllClient(FName SessionName) override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Session|Functions")
	virtual bool FindSession() override;
	virtual void OnFindSessionCompleted(bool bWasSuccessful) override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Session|Functions")
	virtual void JoinSession() override;
	virtual void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;

	UFUNCTION(Category = "EOS|Matchmaking|Functions")
	virtual bool FindMatchMaking() override;
	virtual void OnFindMatchMakingCompleted(bool bWasSuccessful) override;

	UFUNCTION(Category = "EOS|Matchmaking|Functions")
	virtual void JoinMatchMaking() override;
	virtual bool JoinMatchMaking(FName SessionName, const FOnlineSessionSearchResult& OnlineSessionSearchResult);
	virtual void OnJoinMatchMakingCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result) override;

	UFUNCTION(Category = "EOS|Matchmakin|Functions")
	virtual bool FindMatchMakingWithLobby();
	virtual void OnFindMatchMakingWithLobby(bool bWasSuccessful);

	UFUNCTION(Category = "EOS|Matchmaking|Functions")
	virtual void JoinMatchMakingWithLobby();
	virtual void OnJoinMatchMakingWithLobbyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(BlueprintCallable, Category = "EOS|Matchmaking|Functions")
	virtual void MatchMaking() override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Session|Functions")
	virtual void QuitSession(FName SessionName) override;

	UFUNCTION(BlueprintCallable, Category = "EOS|Session|Functions")
	void UpdateSessionStatus(const FString& Status, const FName& SessionName);

	UFUNCTION(BlueprintCallable)
	void EndSession();
	UFUNCTION()
	void StartSession();

	UPROPERTY(BlueprintAssignable)
	FOnCreateSessionComplete OnCreateSessionComplete;

	UPROPERTY(BlueprintAssignable)
	FOnDestroySessionComplete OnDestroySessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "EOS|Matchmaking")
	FOnMatchMakingProgress OnMatchMakingProgressDelegate;
	UPROPERTY(BlueprintAssignable, Category = "EOS|Matchmaking")
	FOnMatchMakingJoining OnMatchMakingJoiningDelegate;

	UFUNCTION(BlueprintCallable)
	FSessionInfo GetSessionInfo();

private:
	IOnlineSessionPtr m_SessionPtr;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY()
	int32 m_MaxFindMatchmakingAttempt = 10;
	UPROPERTY()
	int32 m_FindMatchmakingAttempt = 0;

	UPROPERTY()
	bool m_VerifSessionOptions = false;

	FDelegateHandle FindFriendSessionHandle;

	UPROPERTY()
	UCacheSubsystem* CacheSubsystem;

	UPROPERTY()
	FSessionInfo SessionInfo;
};

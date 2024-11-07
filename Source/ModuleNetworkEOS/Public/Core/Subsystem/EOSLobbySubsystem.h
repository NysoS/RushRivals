// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/Session/LobbyInfo.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSLobbySubsystem.generated.h"

class UCacheSubsystem;
class FOnlineUserPresence;

UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API UEOSLobbySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLobbyCreateCompleted, FName, SessionName, bool, bWasSuccessful);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJoinLobby);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeftLobby, const FName&, SessionName, const FString&, UserName);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyUpdate);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyDestroyComplete);

public:
	static constexpr int32 MAX_LOBBY_MEMBERS = 2;
	static constexpr const char* CACHE_LOBBY_INFO = "C_LOBBY_INFO";

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void BindLobbyDelegate();

	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby|Functions")
	void CreateLobby();
	void OnCreateLobbyProgress(FName SessionName, bool bWasSuccessful);

	void OnLobbyInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	void OnJoinLobbyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(BlueprintCallable)
	bool DestroyLobby(FName SessionName);
	void OnDestroyLobbyProgress(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "EOS|Lobby|Functions")
	void FindSessionById(const FString& FriendId, const FString& LobbyId);
	void OnFindSessionById(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResults);

	UFUNCTION(BlueprintCallable)
	void BackToLobby();

	UFUNCTION(BlueprintCallable)
	void QuitLooby();

	UFUNCTION(BlueprintCallable)
	void Invite(const FString& FriendId);

	UPROPERTY(BlueprintAssignable)
	FOnLobbyCreateCompleted OnCreateLobbyCreateCompleted;

	UFUNCTION(BlueprintCallable)
	bool IsLobbyAlreadyExist() const;

	////////////////
	void OnUpdateLobbySettings(FName SessionName, const FOnlineSessionSettings& SessionSettings);

	//Temp
	UFUNCTION(BlueprintCallable, Category = "TEMP")
	void UpdateSession();

	void OnJoinLobby(FName SessionName, const FUniqueNetId& UserId);
	void OnLeftLobby(FName SessionName, const FUniqueNetId& UserId, EOnSessionParticipantLeftReason Reason);

	UPROPERTY(BlueprintAssignable)
	FOnJoinLobby OnJoinLobbyDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnLeftLobby OnLeftLobbyDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnLobbyUpdate OnLobbyUpdate;

	UPROPERTY(BlueprintAssignable)
	FOnLobbyDestroyComplete OnLobbyDestroyComplete;

	UFUNCTION()
	void OnLeftLobbyDelegateHandle(const FName& SessionName, const FString& UserName);

	UFUNCTION(BlueprintCallable)
	FLobbyInfo GetLobbyInfo();

protected:
	bool bIsLobbyAlreadyCreated = false;
	FDelegateHandle Handle;

	FDelegateHandle UpdateSettingHandle;

	FDelegateHandle JoinLobbyHandle;
	FDelegateHandle DestroyLobbyHandle;

	UPROPERTY()
	UCacheSubsystem* CacheSubsystem;

	UPROPERTY()
	FLobbyInfo LobbyInfo;
};
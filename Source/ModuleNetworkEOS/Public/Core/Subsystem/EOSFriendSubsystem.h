// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EOSFriendSubsystem.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FFriendData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;
};


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MODULENETWORKEOS_API UEOSFriendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetFriendList, const TArray<FFriendData>&, FriendList);

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void OnInviteAccepted(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);

	UFUNCTION(BlueprintCallable)
	void Init();

	UFUNCTION(BlueprintCallable, Category = "EOS|Friend|Functions")
	void OpenOverlay();

	UFUNCTION(BlueprintCallable)
	void Read();
	void OnReadComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	UPROPERTY(BlueprintAssignable)
	FOnGetFriendList OnGetFriendList;

protected:
	TArray<TSharedRef<FOnlineFriend>> FriendsArr;
};

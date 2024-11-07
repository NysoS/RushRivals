// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystem/EOSFriendSubsystem.h"

#include "ModuleNetworkEOSModules.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"

void UEOSFriendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

//void UEOSFriendSubsystem::OnInviteAccepted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId,
//	const FString& ListName, const FString& ErrorStr)
//{
//}

void UEOSFriendSubsystem::OnInviteAccepted(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Custom Invite Accepted callback"));
}

void UEOSFriendSubsystem::Init()
{
}

void UEOSFriendSubsystem::OpenOverlay()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface())
		{
			ExternalUI->ShowFriendsUI(0);
		}
	}
}

void UEOSFriendSubsystem::Read()
{
	if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineFriendsPtr FriendsPtr = Subsystem->GetFriendsInterface())
		{
			FriendsPtr->ReadFriendsList(0, TEXT(""), FOnReadFriendsListComplete::CreateUObject(this, &UEOSFriendSubsystem::OnReadComplete));
		}
	}
}

void UEOSFriendSubsystem::OnReadComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (bWasSuccessful)
	{
		if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
		{
			if (IOnlineFriendsPtr FriendsPtr = Subsystem->GetFriendsInterface())
			{
				FriendsPtr->GetFriendsList(
					0 /* LocalUserNum */,
					TEXT("") /* ListName, unused by EOS */,
					FriendsArr /* OutFriends */
				);

				TArray<FFriendData> FriendData;
				for (const auto& Friend : FriendsArr)
				{
					FFriendData Data;
					Data.UserId = Friend->GetUserId()->ToString();
					Data.DisplayName = Friend->GetDisplayName();

					FriendData.Add(Data);
					UE_LOG(ModuleNetworkEOS, Warning, TEXT("Friend %s on List"), *Friend->GetDisplayName());
				}

				OnGetFriendList.Broadcast(FriendData);
				FriendsArr.Reset();
			}
		}
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystem/EOSLobbySubsystem.h"

#include "Cache/CacheSubsystem.h"
#include "ModuleNetworkEOSModules.h"
#include "OnlineSubsystemUtils.h"
#include "IOnlineSubsystemEOS.h"
#include "OnlineSessionSettings.h"
#include "Core/Library/NetworkEOSBPFunctionLibrary.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Core/GameState/EOSLobbyGameState.h"
#include "Network/Domain/UseCase/GetResolvedAddress.h"
#include "Online/Session/Settings/UpdateActions/SessionSettingActions.h"
#include "Settings/ModuleNetworkEOSSettings.h"
#include "Online/Session/LobbyInfo.h"


class AEOSLobbyGameMode;

void UEOSLobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!EOSOnlineSubsytem)
		return;

	m_SessionPtr = EOSOnlineSubsytem->GetSessionInterface();
	if (!m_SessionPtr)
	{
		ERROR_LOG(TEXT("Session Interface cannot be instanciate"));
	}

	CacheSubsystem = UGameInstance::GetSubsystem<UCacheSubsystem>(GetGameInstance());
	if (!CacheSubsystem)
	{
		ERROR_LOG(TEXT("Cache Subsytem can't be instanciate to EOSLobbySubsystem"));
	}

	BindLobbyDelegate();
}

void UEOSLobbySubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (!m_SessionPtr)
		return;

	m_SessionPtr->ClearOnSessionSettingsUpdatedDelegate_Handle(UpdateSettingHandle);

	m_SessionPtr->ClearOnSessionParticipantJoinedDelegates(this);
	m_SessionPtr->ClearOnSessionParticipantLeftDelegates(this);

	OnLeftLobbyDelegate.RemoveDynamic(this, &UEOSLobbySubsystem::OnLeftLobbyDelegateHandle);

	QuitLooby();
}

void UEOSLobbySubsystem::BindLobbyDelegate()
{
	if (!m_SessionPtr)
		return;

	m_SessionPtr->AddOnSessionUserInviteAcceptedDelegate_Handle(
		FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UEOSLobbySubsystem::OnLobbyInviteAccepted)
	);

	m_SessionPtr->OnSessionParticipantJoinedDelegates.AddUObject(this, &UEOSLobbySubsystem::OnJoinLobby);
	m_SessionPtr->OnSessionParticipantLeftDelegates.AddUObject(this, &UEOSLobbySubsystem::OnLeftLobby);
	OnLeftLobbyDelegate.AddUniqueDynamic(this, &UEOSLobbySubsystem::OnLeftLobbyDelegateHandle);

	UpdateSettingHandle = m_SessionPtr->AddOnSessionSettingsUpdatedDelegate_Handle(
		FOnSessionSettingsUpdatedDelegate::CreateUObject(this, &UEOSLobbySubsystem::OnUpdateLobbySettings)
	);
}

void UEOSLobbySubsystem::CreateLobby()
{
	if (!m_SessionPtr)
		return;

	if (LobbyInfo.bValid)
	{
		return;
	}

	int32 UniqueId = UObject::GetUniqueID();
	FName SessionName = FName("Dev_Lobby_" + FString::FromInt(UniqueId));

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.NumPublicConnections = 0;
	SessionSettings.NumPrivateConnections = MAX_LOBBY_MEMBERS;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bAllowJoinInProgress = false;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUsesStats = true;
	SessionSettings.BuildUniqueId = UniqueId;
	SessionSettings.bUsesPresence = true;

	SessionSettings.Set("Dev_Lobby", FString("dev_l"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set("SESSION_NAME", SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

	m_SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSLobbySubsystem::OnCreateLobbyProgress);
	m_SessionPtr->CreateSession(0, SessionName, SessionSettings);
}

void UEOSLobbySubsystem::OnCreateLobbyProgress(FName SessionName, bool bWasSuccessful)
{
	WARNING_LOG(TEXT("LOBBY SESSION NAME AFTER CREATE : %s"), *SessionName.ToString())

	if (bWasSuccessful) {
		
		//GetWorld()->Listen(GetWorld()->GetCurrentLevel()->URL);
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("On Create Lobby Progress Called"));

		LobbyInfo.bValid = true;
		LobbyInfo.bIsOwner = true;
		LobbyInfo.bUseEOSRegister = true;
		LobbyInfo.SessionName = SessionName;

		if (const IOnlineIdentityPtr IdentityPtr = EOSOnlineSubsytem->GetIdentityInterface())
		{
			LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(0));
		}

		OnCreateLobbyCreateCompleted.Broadcast(SessionName, bWasSuccessful);

		m_SessionPtr->ClearOnCreateSessionCompleteDelegates(this);

		OnCreateLobbyCreateCompleted.Broadcast(SessionName, bWasSuccessful);
	}
}

void UEOSLobbySubsystem::OnLobbyInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!m_SessionPtr)
		return;

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Session invite Accepted"));
	if (bWasSuccessful)
	{
		JoinLobbyHandle = m_SessionPtr->AddOnJoinSessionCompleteDelegate_Handle(
			FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::OnJoinLobbyComplete)
		);

		LobbyInfo.bValid = true;
		LobbyInfo.bIsOwner = false;
		LobbyInfo.bUseEOSRegister = false;

		if (const IOnlineIdentityPtr IdentityPtr = EOSOnlineSubsytem->GetIdentityInterface())
		{
			WARNING_LOG(TEXT("Oui %s"), *IdentityPtr->GetPlayerNickname(*UserId));
			LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(*UserId));
			LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(0));
		}

		FString SessionName;
		InviteResult.Session.SessionSettings.Get("SESSION_NAME", SessionName);
		WARNING_LOG(TEXT("LOBBY INVITE SESSION NAME %s"), *SessionName)
		LobbyInfo.SessionName = FName(SessionName);

		m_SessionPtr->JoinSession(0, FName(SessionName), InviteResult);
	}

	m_SessionPtr->ClearOnSessionInviteReceivedDelegate_Handle(Handle);
	bIsLobbyAlreadyCreated = true;
}

void UEOSLobbySubsystem::OnJoinLobbyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		OnJoinLobbyDelegate.Broadcast();
		m_SessionPtr->ClearOnJoinSessionCompleteDelegate_Handle(JoinLobbyHandle);
	}
}

bool UEOSLobbySubsystem::DestroyLobby(FName SessionName)
{
	if (!m_SessionPtr)
		return false;

	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	OnDestroySessionCompleteDelegate.BindUObject(this, &UEOSLobbySubsystem::OnDestroyLobbyProgress);

	if (LobbyInfo.bIsOwner)
	{
		FOnlineSessionSettings* LobbySettings = m_SessionPtr->GetSessionSettings(SessionName);
		LobbySettings->Set(FName("setting_update_action_type"), FString("DestroyLobby"), EOnlineDataAdvertisementType::ViaOnlineService);
		m_SessionPtr->UpdateSession(SessionName, *LobbySettings);
	}

	DestroyLobbyHandle = m_SessionPtr->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
	return m_SessionPtr->DestroySession(SessionName, OnDestroySessionCompleteDelegate);
}

void UEOSLobbySubsystem::OnDestroyLobbyProgress(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful) {

		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Destroy Success"));

		OnLeftLobbyDelegate.Broadcast(SessionName, "");
		LobbyInfo.Clear();
		OnLobbyDestroyComplete.Broadcast();
	}
}

void UEOSLobbySubsystem::BackToLobby()
{
	if (!m_SessionPtr)
		return;

	if (LobbyInfo.bIsOwner)
	{
		FOnlineSessionSettings* LobbySettings = m_SessionPtr->GetSessionSettings(LobbyInfo.SessionName);
		LobbySettings->Set(FName("setting_update_action_type"), FString("BackLobby"), EOnlineDataAdvertisementType::ViaOnlineService);
		m_SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);
	}

	UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetGameInstance());
	if (!SessionSubsystem)
	{
		WARNING_LOG(TEXT("NO SESSION SUBSYSTEM"));
		return;
	}

	FSessionInfo SessionInfo;
	SessionInfo = SessionSubsystem->GetSessionInfo();
	
	WARNING_LOG(TEXT("SESSION REGISTER TO %s"), *SessionInfo.SessionName.ToString());
	if (SessionInfo.bUseEOSRegister)
	{
		WARNING_LOG(TEXT("DESTROY SESSION BACK TO LOBBY"));
		SessionSubsystem->DestroySession(SessionInfo.SessionName);
		return;
	}

	if (!LobbyInfo.bUseEOSRegister)
	{
		if (const UModuleNetworkEOSSettings* ConnectSetting = Cast<UModuleNetworkEOSSettings>(UModuleNetworkEOSSettings::StaticClass()->GetDefaultObject(false)))
		{
			UGameplayStatics::OpenLevel(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), FName(ConnectSetting->DefaultMenuMapText.GetAssetName()));
		}
	}
}

void UEOSLobbySubsystem::QuitLooby()
{
	if (!m_SessionPtr)
		return;

	if (!LobbyInfo.bValid)
	{
		return;
	}

	DestroyLobby(FName(LobbyInfo.SessionName));
}

void UEOSLobbySubsystem::Invite(const FString& FriendId)
{
	if (!m_SessionPtr)
		return;

	if (!LobbyInfo.bValid)
	{
		return;
	}

	if (const IOnlineIdentityPtr IdentityPtr = EOSOnlineSubsytem->GetIdentityInterface())
	{
		TSharedPtr<const FUniqueNetId> FriendNetId = IdentityPtr->CreateUniquePlayerId(FriendId);

		if (FriendNetId != nullptr)
		{
			if (m_SessionPtr->SendSessionInviteToFriend(0, LobbyInfo.SessionName, *FriendNetId))
			{
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("Invite Successfully"));
			}
			else
			{
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("Invite Error"));
			}
		}
	}
}

bool UEOSLobbySubsystem::IsLobbyAlreadyExist() const
{
	return bIsLobbyAlreadyCreated;
}

void UEOSLobbySubsystem::OnUpdateLobbySettings(FName SessionName, const FOnlineSessionSettings& SessionSettings)
{
	WARNING_LOG(TEXT("Lobby Updated"));

	OnLobbyUpdate.Broadcast();

	FString UpdateActionType;
	if (!SessionSettings.Get(FName("setting_update_action_type"), UpdateActionType))
	{
		return;
	}

	UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetGameInstance());
	WARNING_LOG(TEXT("Value %s"), *UpdateActionType);

	if (UpdateActionType == SearchMatchmakingAction::GetClassName())
	{
		OnSettingUpdateAction<SearchMatchmakingAction>::execute(SessionSubsystem);
	} else if (UpdateActionType == JoinMatchmakingAction::GetClassName())
	{
		OnSettingUpdateAction<JoinMatchmakingAction>::execute(SessionSubsystem);
	} else if (UpdateActionType == TravelMatchmakingAction::GetClassName())
	{
		FString ServerAddress, ServerParams, SessionNameInfo;
		SessionSettings.Get(FName("server_address"), ServerAddress);
		SessionSettings.Get(FName("server_params"), ServerParams);
		SessionSettings.Get(FName("session_name"), SessionNameInfo);

		FSessionInfo SessionInfo = SessionSubsystem->GetSessionInfo();
		SessionInfo.SessionName = FName(SessionNameInfo);
		SessionInfo.bValid = true;
		SessionInfo.bUseEOSRegister = false;

		Request request = Request(ServerAddress, ServerParams);
		OnSettingUpdateAction<TravelMatchmakingAction>::execute(request, GetWorld());
	} else if (UpdateActionType == "DestroyLobby")
	{
		WARNING_LOG(TEXT("DESTROYLOBBY SESSIONName: %s"), *SessionName.ToString());

		if (!LobbyInfo.bIsOwner)
		{
			QuitLooby();
		}
	}
	else if (UpdateActionType == "BackLobby") {
		OnJoinLobbyDelegate.Broadcast();
	}
}

void UEOSLobbySubsystem::UpdateSession()
{
	if (!m_SessionPtr)
		return;

	FOnlineSessionSettings* SearchSettings = m_SessionPtr->GetSessionSettings(FName("Dev_Lobby"));
	if (SearchSettings)
	{
		SearchSettings->Set(FName("SUAT"), FString("UPDATED VALUE"), EOnlineDataAdvertisementType::ViaOnlineService);
	}
	m_SessionPtr->UpdateSession(FName("Dev_Lobby"), *SearchSettings);
}

void UEOSLobbySubsystem::OnJoinLobby(FName SessionName, const FUniqueNetId& UserId)
{
	WARNING_LOG(TEXT("JoinLobby"));

	if (const IOnlineIdentityPtr IdentityPtr = EOSOnlineSubsytem->GetIdentityInterface())
	{
		WARNING_LOG(TEXT("Join %s"), *IdentityPtr->GetPlayerNickname(UserId))

		LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(UserId));
	}

	OnJoinLobbyDelegate.Broadcast();
}

void UEOSLobbySubsystem::OnLeftLobby(FName SessionName, const FUniqueNetId& UserId, EOnSessionParticipantLeftReason Reason)
{
	WARNING_LOG(TEXT("LeftLobby"));

	FString UserToLeft;
	if (const IOnlineIdentityPtr IdentityPtr = EOSOnlineSubsytem->GetIdentityInterface())
	{
		UserToLeft = IdentityPtr->GetPlayerNickname(UserId);
	}

	OnLeftLobbyDelegate.Broadcast(SessionName, UserToLeft);
}

void UEOSLobbySubsystem::OnLeftLobbyDelegateHandle(const FName& SessionName, const FString& UserName)
{
	for (auto Element : LobbyInfo.membersName)
	{
		if (Element == UserName)
		{
			LobbyInfo.membersName.Remove(Element);
			break;
		}
	}
}

FLobbyInfo UEOSLobbySubsystem::GetLobbyInfo()
{
	return LobbyInfo;
}

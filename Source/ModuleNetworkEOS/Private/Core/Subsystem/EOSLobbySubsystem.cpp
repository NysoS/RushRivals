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

//	REGISTER_ACTION_TYPE(SearchMatchmakingAction)


	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			SessionPtr->AddOnSessionUserInviteAcceptedDelegate_Handle(
				FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UEOSLobbySubsystem::OnLobbyInviteAccepted)
			);

			SessionPtr->OnSessionParticipantJoinedDelegates.AddUObject(this, &UEOSLobbySubsystem::OnJoinLobby);
			SessionPtr->OnSessionParticipantLeftDelegates.AddUObject(this, &UEOSLobbySubsystem::OnLeftLobby);
			OnLeftLobbyDelegate.AddUniqueDynamic(this, &UEOSLobbySubsystem::OnLeftLobbyDelegateHandle);
			UE_LOG(ModuleNetworkEOS, Warning, TEXT("Handle Received"));
		}
	}

	CacheSubsystem = UGameInstance::GetSubsystem<UCacheSubsystem>(GetGameInstance());
	if (!CacheSubsystem)
	{
		ERROR_LOG(TEXT("Cache Subsytem can't be instanciate to %s"), ThisClass::StaticClass())
	}

	BindLobbyDelegate();
}

void UEOSLobbySubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
			SessionPtr->ClearOnSessionSettingsUpdatedDelegate_Handle(UpdateSettingHandle);


			SessionPtr->ClearOnSessionParticipantJoinedDelegates(this);
			SessionPtr->ClearOnSessionParticipantLeftDelegates(this);

			OnLeftLobbyDelegate.RemoveDynamic(this, &UEOSLobbySubsystem::OnLeftLobbyDelegateHandle);
		}
	}

	QuitLooby();
}

void UEOSLobbySubsystem::BindLobbyDelegate()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
			UpdateSettingHandle = SessionPtr->AddOnSessionSettingsUpdatedDelegate_Handle(
				FOnSessionSettingsUpdatedDelegate::CreateUObject(this, &UEOSLobbySubsystem::OnUpdateLobbySettings)
			);
		}
	}
}

void UEOSLobbySubsystem::CreateLobby()
{
	if (LobbyInfo.bValid)
	{
		return;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {

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

			/*LobbyInfo.bIsOwner = true;
			LobbyInfo.bUseEOSRegister = true;
			LobbyInfo.SessionName = SessionName;

			if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
			{
				LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(0));
			}*/

			/*if (CacheSubsystem)
			{
				CacheSubsystem->Set(CACHE_LOBBY_INFO, LobbyInfo);
			}*/

			SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSLobbySubsystem::OnCreateLobbyProgress);
			SessionPtr->CreateSession(0, SessionName, SessionSettings);
		}
	}
}

void UEOSLobbySubsystem::OnCreateLobbyProgress(FName SessionName, bool bWasSuccessful)
{
	WARNING_LOG(TEXT("LOBBY SESSION NAME AFTER CREATE : %s"), *SessionName.ToString())

	if (bWasSuccessful) {
		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {

				//GetWorld()->Listen(GetWorld()->GetCurrentLevel()->URL);
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("On Create Lobby Progress Called"));

				LobbyInfo.bValid = true;
				LobbyInfo.bIsOwner = true;
				LobbyInfo.bUseEOSRegister = true;
				LobbyInfo.SessionName = SessionName;

				if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
				{
					LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(0));
				}

				OnCreateLobbyCreateCompleted.Broadcast(SessionName, bWasSuccessful);

				SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
			/*	bIsLobbyAlreadyCreated = true;
				LobbyGameState->bIsLobbyActive = true;

				const FString LobbyId = GetSessionId::invoke(SessionPtr, SessionName);
				LobbyGameState->SetLoobyId(LobbyId);*/

				//const Request request = GetResolvedAddress<DefaultResolverType>::invoke(SessionPtr, SessionName);
		/*		const FRequestDTO RequestDto = Request::makeDTO(request);

				FLobbyInfo LobbyInfo;
				LobbyInfo.bIsOwner = true;
				LobbyInfo.LobbyId = LobbyId;
				LobbyInfo.RequestDto = RequestDto;
				LobbyInfo.SessionName = SessionName;

				WARNING_LOG(TEXT("Request URL %s"), *request.getUrl())
				CacheSubsystem->Set(FString(CACHE_LOBBY_INFO), LobbyInfo);*/

				//CacheSubsystem->Set("CACHE_LOBBY_ID", FCacheValue(LobbyId));
			}
		}

		//FCacheValue LobbyIdCacheValue = CacheSubsystem->Get("CACHE_LOBBY_ID");
		//WARNING_LOG(TEXT("Get LobbyId Cache: ID - %s"), LobbyIdCacheValue.value);

		OnCreateLobbyCreateCompleted.Broadcast(SessionName, bWasSuccessful);
	}
}

void UEOSLobbySubsystem::OnLobbyInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Session invite Accepted"));
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) 
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) 
		{
			if (bWasSuccessful)
			{
				JoinLobbyHandle = SessionPtr->AddOnJoinSessionCompleteDelegate_Handle(
					FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::OnJoinLobbyComplete)
				);

				LobbyInfo.bValid = true;
				LobbyInfo.bIsOwner = false;
				LobbyInfo.bUseEOSRegister = false;

				if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
				{
					WARNING_LOG(TEXT("Oui %s"), *IdentityPtr->GetPlayerNickname(*UserId));
					LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(*UserId));
					LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(0));
				}

				FString SessionName;
				InviteResult.Session.SessionSettings.Get("SESSION_NAME", SessionName);
				WARNING_LOG(TEXT("LOBBY INVITE SESSION NAME %s"), *SessionName)
				LobbyInfo.SessionName = FName(SessionName);
				//CacheSubsystem->Set(FString(CACHE_LOBBY_INFO), LobbyInfo);

				SessionPtr->JoinSession(0, FName(SessionName), InviteResult);
			}

			SessionPtr->ClearOnSessionInviteReceivedDelegate_Handle(Handle);
			bIsLobbyAlreadyCreated = true;
		}
	}
}

void UEOSLobbySubsystem::OnJoinLobbyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		/*if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), 0))
		{*/
			if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
			{
				if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
				{
	/*				const Request request = GetResolvedAddress<DefaultResolverType>::invoke(SessionPtr, SessionName);
					const FString LobbyId = GetSessionId::invoke(SessionPtr, SessionName);
					const FRequestDTO RequestDto = Request::makeDTO(request);

					FLobbyInfo LobbyInfo;
					CacheSubsystem->Get(FString(CACHE_LOBBY_INFO), LobbyInfo);
					LobbyInfo.LobbyId = LobbyId;
					LobbyInfo.RequestDto = RequestDto;
					LobbyInfo.SessionName = SessionName;
						
					CacheSubsystem->Set(FString(CACHE_LOBBY_INFO), LobbyInfo);

					if (request.isValid())
					{
						PlayerController->ClientTravel(request.getUrl(), TRAVEL_Absolute);
					}*/

					OnJoinLobbyDelegate.Broadcast();
					SessionPtr->ClearOnJoinSessionCompleteDelegate_Handle(JoinLobbyHandle);
				}
			}
		//}
	}
}

bool UEOSLobbySubsystem::DestroyLobby(FName SessionName)
{
	if (const IOnlineSubsystem* SubSystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = SubSystem->GetSessionInterface()) {

			FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
			OnDestroySessionCompleteDelegate.BindUObject(this, &UEOSLobbySubsystem::OnDestroyLobbyProgress);

			/*FLobbyInfo LobbyInfo;
			if (CacheSubsystem->Get(CACHE_LOBBY_INFO, LobbyInfo))
			{*/
			if (LobbyInfo.bIsOwner)
			{
				FOnlineSessionSettings* LobbySettings = SessionPtr->GetSessionSettings(SessionName);
				LobbySettings->Set(FName("setting_update_action_type"), FString("DestroyLobby"), EOnlineDataAdvertisementType::ViaOnlineService);
				SessionPtr->UpdateSession(SessionName, *LobbySettings);
			}
			//}

			DestroyLobbyHandle = SessionPtr->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
			return SessionPtr->DestroySession(SessionName, OnDestroySessionCompleteDelegate);
		}
	}

	return false;
}

void UEOSLobbySubsystem::OnDestroyLobbyProgress(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful) {

		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Destroy Success"));

		OnLeftLobbyDelegate.Broadcast(SessionName, "");
		LobbyInfo.Clear();
		OnLobbyDestroyComplete.Broadcast();
		//UGameplayStatics::OpenLevel(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), "Menu");
	}
}

void UEOSLobbySubsystem::FindSessionById(const FString& FriendId, const FString& LobbyId)
{
	//if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
	//	if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
	//	{
	//		if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
	//		{
	//			/*FLobbyInfo LobbyInfo;
	//			CacheSubsystem->Get(FString(CACHE_LOBBY_INFO), LobbyInfo);

	//			FUniqueNetIdPtr UniqueNetId = IdentityPtr->GetUniquePlayerId(0);
	//			FUniqueNetIdPtr OwnerNetId = IdentityPtr->CreateUniquePlayerId(FriendId);
	//			FUniqueNetIdPtr LobbyNetId = IdentityPtr->CreateUniquePlayerId(LobbyId);

	//			FDelegateHandle g = SessionPtr->OnEndSessionCompleteDelegates.AddUObject(this, &UEOSLobbySubsystem::OnCreateLobbyProgress);*/

	//			//SessionPtr->FindSessionById(*UniqueNetId, *LobbyNetId, *OwnerNetId, FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::OnFindSessionById));
	//		}
	//	}
	//}
}

void UEOSLobbySubsystem::OnFindSessionById(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResults)
{
	/*if (bWasSuccessful)
	{
		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
			{
				FLobbyInfo LobbyInfo;
				CacheSubsystem->Get(FString(CACHE_LOBBY_INFO), LobbyInfo);

				JoinLobbyHandle = SessionPtr->AddOnJoinSessionCompleteDelegate_Handle(
					FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSLobbySubsystem::OnJoinLobbyComplete)
				);

				SessionPtr->JoinSession(0, LobbyInfo.SessionName, SearchResults);
			}
		}
	}*/
}

void UEOSLobbySubsystem::BackToLobby()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {

		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			if (LobbyInfo.bIsOwner)
			{
				FOnlineSessionSettings* LobbySettings = SessionPtr->GetSessionSettings(LobbyInfo.SessionName);
				LobbySettings->Set(FName("setting_update_action_type"), FString("BackLobby"), EOnlineDataAdvertisementType::ViaOnlineService);
				SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);
			}
		}
	}

	UEOSSessionSubsystem* SessionSubsystem = UGameInstance::GetSubsystem<UEOSSessionSubsystem>(GetGameInstance());
	if (!SessionSubsystem)
	{
		WARNING_LOG(TEXT("NO SESSION SUBSYSTEM"));
		return;
	}

	/*FLobbyInfo LobbyInfo;
	bool bIsLobbyCached = CacheSubsystem->Get<FLobbyInfo>(FString(UEOSLobbySubsystem::CACHE_LOBBY_INFO), LobbyInfo);*/

	FSessionInfo SessionInfo;
	//if (CacheSubsystem) {
	//	bool bIsSessionCached = CacheSubsystem->Get<FSessionInfo>(UEOSSessionSubsystem::C_SESSION_INFO, SessionInfo);
	//	WARNING_LOG(TEXT("SESSION REGISTER TO %s"), *SessionInfo.SessionName.ToString());
	//}
	//else {
		SessionInfo = SessionSubsystem->GetSessionInfo();
//	}
	
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
	//FLobbyInfo LobbyInfo;
	//bool bIsLobbyCached = CacheSubsystem->Get(CACHE_LOBBY_INFO, LobbyInfo);

	if (!LobbyInfo.bValid)
	{
		return;
	}
	//CacheSubsystem->Clear(CACHE_LOBBY_INFO);

	DestroyLobby(FName(LobbyInfo.SessionName));
}

void UEOSLobbySubsystem::Invite(const FString& FriendId)
{
	/*FLobbyInfo LobbyInfo;
	bool bIsLobbyCached = CacheSubsystem->Get(CACHE_LOBBY_INFO, LobbyInfo);*/

	if (!LobbyInfo.bValid)
	{
		return;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {

		if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
		{
			TSharedPtr<const FUniqueNetId> FriendNetId = IdentityPtr->CreateUniquePlayerId(FriendId);

//			UE_LOG(ModuleNetworkEOS, Warning, TEXT("FriendId %s - UniqueFriendId %s"), *FriendId, *FriendNetId->ToString());

			if (FriendNetId != nullptr)
			{
				if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) 
				{
					if (SessionPtr->SendSessionInviteToFriend(0, LobbyInfo.SessionName, *FriendNetId))
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
		FString ServerAddress, ServerParams, SessionName;
		SessionSettings.Get(FName("server_address"), ServerAddress);
		SessionSettings.Get(FName("server_params"), ServerParams);
		SessionSettings.Get(FName("session_name"), SessionName);

		FSessionInfo SessionInfo = SessionSubsystem->GetSessionInfo();
		SessionInfo.SessionName = FName(SessionName);
		SessionInfo.bValid = true;
		SessionInfo.bUseEOSRegister = false;

		Request request = Request(ServerAddress, ServerParams);
		OnSettingUpdateAction<TravelMatchmakingAction>::execute(request, GetWorld());
	} else if (UpdateActionType == "DestroyLobby")
	{
		//FLobbyInfo LobbyInfo;
		WARNING_LOG(TEXT("DESTROYLOBBY SESSIONName: %s"), *SessionName.ToString());

		if (!LobbyInfo.bIsOwner)
		{
			QuitLooby();
		}

		/*if (CacheSubsystem->Get(CACHE_LOBBY_INFO, LobbyInfo))
		{
			if(!LobbyInfo.bIsOwner)
			{
				QuitLooby();
			}
		}*/
	}
	else if (UpdateActionType == "BackLobby") {
		OnJoinLobbyDelegate.Broadcast();
	}
}

void UEOSLobbySubsystem::UpdateSession()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {

		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			FOnlineSessionSettings* SearchSettings = SessionPtr->GetSessionSettings(FName("Dev_Lobby"));
			if (SearchSettings)
			{
				SearchSettings->Set(FName("SUAT"), FString("UPDATED VALUE"), EOnlineDataAdvertisementType::ViaOnlineService);
			}
			SessionPtr->UpdateSession(FName("Dev_Lobby"), *SearchSettings);
		}
	}
}

void UEOSLobbySubsystem::OnJoinLobby(FName SessionName, const FUniqueNetId& UserId)
{
	WARNING_LOG(TEXT("JoinLobby"));
	//FLobbyInfo LobbyInfo;

	//bool bIsLobbyCached = CacheSubsystem->Get(CACHE_LOBBY_INFO, LobbyInfo);
	//if (bIsLobbyCached)
	//{

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
		{
			auto d = IdentityPtr->CreateUniquePlayerId(UserId.ToString());

			WARNING_LOG(TEXT("Join %s"), *IdentityPtr->GetPlayerNickname(UserId))

			
			LobbyInfo.membersName.Add(IdentityPtr->GetPlayerNickname(UserId));
		}
	}
	
	//}
	//CacheSubsystem->Set(CACHE_LOBBY_INFO, LobbyInfo);

	OnJoinLobbyDelegate.Broadcast();
}

void UEOSLobbySubsystem::OnLeftLobby(FName SessionName, const FUniqueNetId& UserId, EOnSessionParticipantLeftReason Reason)
{
	WARNING_LOG(TEXT("LeftLobby"));

	FString UserToLeft;
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {

		if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
		{
			UserToLeft = IdentityPtr->GetPlayerNickname(UserId);
		}
	}

	OnLeftLobbyDelegate.Broadcast(SessionName, UserToLeft);
}

void UEOSLobbySubsystem::OnLeftLobbyDelegateHandle(const FName& SessionName, const FString& UserName)
{
	//FLobbyInfo LobbyInfo;

	//bool bIsLobbyCached = CacheSubsystem->Get(CACHE_LOBBY_INFO, LobbyInfo);
	//if (bIsLobbyCached)
	//{
	for (auto Element : LobbyInfo.membersName)
	{
		if (Element == UserName)
		{
			LobbyInfo.membersName.Remove(Element);
			break;
		}
	}
	//}
	//CacheSubsystem->Set(CACHE_LOBBY_INFO, LobbyInfo);
}

FLobbyInfo UEOSLobbySubsystem::GetLobbyInfo()
{
	return LobbyInfo;
}

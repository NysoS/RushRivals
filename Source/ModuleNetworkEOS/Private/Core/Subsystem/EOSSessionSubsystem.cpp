// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Settings/ModuleNetworkEOSSettings.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Core/Library/NetworkEOSBPFunctionLibrary.h"
#include "ModuleNetworkEOSModules.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"

#if WITH_EDITOR
#include "UnrealEd.h"
#endif

#include "Core/GameInstance/EOSGameInstance.h"
#include "Core/GameState/EOSLobbyGameState.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "Core/Subsystem/EOSLobbySubsystem.h"
#include "Network/Domain/UseCase/GetResolvedAddress.h"
#include "Cache/CacheSubsystem.h"
#include "Network/Domain/UseCase/GetSessionId.h"
#include "Online/Session/LobbyInfo.h"
#include "Online/Session/Settings/UpdateActions/Types/SessionSettingActionType.h"

void UEOSSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CacheSubsystem = UGameInstance::GetSubsystem<UCacheSubsystem>(GetGameInstance());
	if (!CacheSubsystem)
	{
		ERROR_LOG(TEXT("Cache Subsytem can't be instanciate to %s"), ThisClass::StaticClass())
	}
}

void UEOSSessionSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetOuterUGameInstance());
	if (!GameInstance)
	{
		return;
	}

	if (GameInstance->NetworkData.SessionState != Session_None)
	{
		bool bIsDestroyed = false;
		if (GameInstance->NetworkData.NetRole == ROLE_Authority)
		{
			bIsDestroyed = DestroySessionAllClient(GameInstance->NetworkData.SessionName);
		}
		else
		{
			bIsDestroyed = DestroySession(GameInstance->NetworkData.SessionName);
		}

		if (bIsDestroyed)
		{
			GameInstance->NetworkData.SessionName = FName("");
			GameInstance->NetworkData.SessionState = Session_None;
			GameInstance->NetworkData.NetRole = ROLE_None;
		}
	}
}

void UEOSSessionSubsystem::CreateSession(bool bIsDedicateServer, int32 NumberOfPublicConnection)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {

			int32 UniqueId = UObject::GetUniqueID();

			FOnlineSessionSettings SessionSettings;
			SessionSettings.bIsDedicated = bIsDedicateServer;
			SessionSettings.bIsLANMatch = false;
			SessionSettings.NumPublicConnections = NumberOfPublicConnection;
			SessionSettings.bAllowInvites = false;
			SessionSettings.bAllowJoinInProgress = false;
			SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bUseLobbiesIfAvailable = false;
			SessionSettings.bUsesStats = true;
			SessionSettings.BuildUniqueId = UniqueId;
			SessionSettings.bUsesPresence = true;

			SessionSettings.Set("Dev_Session", FString("dev"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set("Max_Players", NumberOfPublicConnection, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

			SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnCreateSessionProgress);
			SessionPtr->CreateSession(0, FName("Dev_Session"), SessionSettings);
		}
	}
}

void UEOSSessionSubsystem::OnCreateSessionProgress(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful) {

		UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetOuterUGameInstance());
		if (!GameInstance)
		{
			return;
		}
		GameInstance->NetworkData.NetRole = ROLE_Authority;
		GameInstance->NetworkData.SessionState = Joined;
		GameInstance->NetworkData.SessionName = SessionName;
		GameInstance->NetworkData.ServerType = P2P;

		UE_LOG(ModuleNetworkEOS, Warning, TEXT("PORT : %i"), GetWorld()->URL.Port);

		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {

				FOnlineSessionSettings* SessionSettings = SessionPtr->GetSessionSettings(SessionName);
				if (SessionSettings)
				{
					SessionSettings->Set(FName(SESSION_SETTINGS_PORT), FString::FromInt(GetWorld()->URL.Port), EOnlineDataAdvertisementType::ViaOnlineService);
					SessionPtr->UpdateSession(SessionName, *SessionSettings);
				}

				SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
			}
		}

		if (const UModuleNetworkEOSSettings* ConnectSetting = Cast<UModuleNetworkEOSSettings>(UModuleNetworkEOSSettings::StaticClass()->GetDefaultObject(false))) {
			if (UWorld* World = UNetworkEOSBPFunctionLibrary::GetActiveWorld())
			{
				World->ServerTravel(ConnectSetting->DefaultSessionMapText.GetAssetName()+"?listen", true);
			}
			else
			{
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("[UEOS_Session] : Not worldContext"));
			}
		}

		OnCreateSessionComplete.Broadcast(SessionName, bWasSuccessful);
	}
}

void UEOSSessionSubsystem::CreateSessionSever(int32 MaxPlayer)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {

			const int32 UniqueId = GetUniqueID();
			FName SessionName = FName("Dev_Session_" + FString::FromInt(UniqueId));

			FOnlineSessionSettings SessionSettings;
			SessionSettings.bIsDedicated = true;
			SessionSettings.bIsLANMatch = false;
			SessionSettings.NumPublicConnections = MaxPlayer;
			SessionSettings.bAllowInvites = false;
			SessionSettings.bAllowJoinInProgress = false;
			SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bUseLobbiesIfAvailable = false;
			SessionSettings.bUsesStats = true;
			SessionSettings.BuildUniqueId = UniqueId;
			SessionSettings.bUsesPresence = false;

			SessionSettings.Set(FName(SESSION_SETTINGS_SHOW_ALL_SESSIONS), FString(SESSION_SETTINGS_SHOW_ALL_SESSIONS_VALUE), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			SessionSettings.Set(FName(SESSION_SETTINGS_SESSION_NAME), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

			FOnCreateSessionCompleteDelegate OnCreateSessionServerCompleteDelegates;
			OnCreateSessionServerCompleteDelegates.BindUObject(this, &UEOSSessionSubsystem::OnCreateSessionServerProgress);

			SessionPtr->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionServerCompleteDelegates);
			SessionPtr->CreateSession(0, SessionName, SessionSettings);
		}
	}
}

void UEOSSessionSubsystem::OnCreateSessionServerProgress(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful) {

		UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetOuterUGameInstance());
		if (!GameInstance)
		{
			return;
		}
		GameInstance->NetworkData.NetRole = ROLE_Authority;
		GameInstance->NetworkData.SessionState = Joined;
		GameInstance->NetworkData.SessionName = SessionName;
		GameInstance->NetworkData.ServerType = DedicatedServer;

		SessionInfo.SessionName = SessionName;

		WARNING_LOG(TEXT("Session Name: %s"), *SessionName.ToString())
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("PORT : %i"), GetWorld()->URL.Port);

		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {

				FOnlineSessionSettings* SessionSettings = SessionPtr->GetSessionSettings(SessionName);
				if (SessionSettings)
				{
					SessionSettings->Set(FName(SESSION_SETTINGS_PORT), FString::FromInt(GetWorld()->URL.Port), EOnlineDataAdvertisementType::ViaOnlineService);
					SessionPtr->UpdateSession(SessionName, *SessionSettings);
				}

				SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
			}
		}

		OnCreateSessionComplete.Broadcast(SessionName, bWasSuccessful);
	}
}

bool UEOSSessionSubsystem::DestroySession(FName SessionName)
{
	if (const IOnlineSubsystem* SubSystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = SubSystem->GetSessionInterface()) {
			FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
			OnDestroySessionCompleteDelegate.BindUObject(this, &UEOSSessionSubsystem::OnDestroySessionProgress);

			SessionPtr->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
			return SessionPtr->DestroySession(SessionName, OnDestroySessionCompleteDelegate);
		}
	}

	return false;
}

bool UEOSSessionSubsystem::DestroySessionAllClient(FName SessionName)
{
	UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetOuterUGameInstance());
	if (!GameInstance)
	{
		return false;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			return SessionPtr->DestroySession(GameInstance->NetworkData.SessionName);
		}
	}

	return false;
}

void UEOSSessionSubsystem::OnDestroySessionProgress(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful) {

		WARNING_LOG(TEXT("Destroy %s Success"), *SessionName.ToString())
		OnDestroySessionComplete.Broadcast(SessionName, bWasSuccessful);

		SessionInfo.clear();

		if (const UModuleNetworkEOSSettings* ConnectSetting = Cast<UModuleNetworkEOSSettings>(UModuleNetworkEOSSettings::StaticClass()->GetDefaultObject(false)))
		{
			UGameplayStatics::OpenLevel(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), FName(ConnectSetting->DefaultMenuMapText.GetAssetName()));
		}

		/*APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (!PlayerController)
		{
			return;
		}

		FLobbyInfo LobbyInfo;
		bool cacheValid = CacheSubsystem->Get<FLobbyInfo>(FString(UEOSLobbySubsystem::CACHE_LOBBY_INFO), LobbyInfo);
		
		if (cacheValid)
		{
			Request request = Request::makeRequestFromDTO(LobbyInfo.RequestDto);
			if (!request.isValid())
			{
				UGameplayStatics::OpenLevel(GetWorld(), "L_Menu");
			}

			UGameplayStatics::OpenLevel(GetWorld(), "L_Menu?listen");
		}*/
	}
}

bool UEOSSessionSubsystem::FindSession()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			UE_LOG(ModuleNetworkEOS, Warning, TEXT("Search in progress...!"));

			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->QuerySettings.SearchParams.Empty();

			SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnFindSessionCompleted);
			return SessionPtr->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}
	return false;
}

void UEOSSessionSubsystem::OnFindSessionCompleted(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
				SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
			}
		}

		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Number of session finded : %i"), SessionSearch.Get()->SearchResults.Num());
	}
}

void UEOSSessionSubsystem::JoinSession()
{
	if(!SessionSearch.IsValid())
	{
		return;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			if (SessionSearch.Get()->SearchResults.Num() > 0)
			{
				SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnJoinSessionCompleted);
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server found"));

				SessionPtr->JoinSession(0, FName("Dev_Session"), SessionSearch->SearchResults[0]);
				SessionSearch.Reset();
			}
			else
			{
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server not found!"));
			}
		}
	}
}

void UEOSSessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Session %s joined callback"), *SessionName.ToString());
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), 0))
		{
			if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
			{
				FString JoinAddress;
				if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
				{
					SessionPtr->GetResolvedConnectString(SessionName, JoinAddress);

					UE_LOG(ModuleNetworkEOS, Warning, TEXT("Join adresse is %s"), *JoinAddress);
					if (!JoinAddress.IsEmpty())
					{
						JoinAddress = JoinAddress + FString("?lobbyid=iu");
						UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetOuterUGameInstance());
						if (!GameInstance)
						{
							UE_LOG(ModuleNetworkEOS, Warning, TEXT("GameInstance null"));
							return;
						}
						GameInstance->NetworkData.SessionName = SessionName;
						GameInstance->NetworkData.SessionState = ESessionState::Joined;

						UE_LOG(ModuleNetworkEOS, Warning, TEXT("Travel player"));
						PlayerController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
					}
					SessionPtr->ClearOnJoinSessionCompleteDelegates(this);
				}
			}
		}
	}
}

bool UEOSSessionSubsystem::FindMatchMaking()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			UE_LOG(ModuleNetworkEOS, Warning, TEXT("Search in progress...!"));
			
			OnMatchMakingProgressDelegate.Broadcast();

			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->TimeoutInSeconds = 60.f;
			SessionSearch->QuerySettings.Set(SEARCH_MINSLOTSAVAILABLE, 1, EOnlineComparisonOp::GreaterThanEquals);

			//SessionSearch->QuerySettings.Set(EOS_SESSIONS_SEARCH_MINSLOTSAVAILABLE, 2, EOnlineComparisonOp::LessThan);

			//TODO: ajouter une contrainte pour rechercher uniquement les serveurs dédier
			SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnFindMatchMakingCompleted);
			return SessionPtr->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}
	return false;
}

void UEOSSessionSubsystem::OnFindMatchMakingCompleted(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
				SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
			}
		}
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Number of session finded : %i"), SessionSearch.Get()->SearchResults.Num());

		if(SessionSearch.Get()->SearchResults.Num() <= 0)
		{
			FindMatchMaking();
		}
		else
		{
			JoinMatchMaking();
		}
	}else
	{
		FindMatchMaking();
	}
}

void UEOSSessionSubsystem::JoinMatchMaking()
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Connexion in progress...!"));
	if (!SessionSearch.IsValid())
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("SessionSearch is not valid!"));
		return;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
	{
		if (IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			if (SessionSearch.Get()->SearchResults.Num() > 0)
			{
				m_FindMatchmakingAttempt = 0;

			/*	FLobbyInfo LobbyInfo;
				bool bIsLobbyCached = CacheSubsystem->Get(UEOSLobbySubsystem::CACHE_LOBBY_INFO, LobbyInfo);
				if (!bIsLobbyCached)
				{
					return;
				}*/

				OnMatchMakingJoiningDelegate.Broadcast();

				/*FOnlineSessionSettings* LobbySettings = SessionPtr->GetSessionSettings(LobbyInfo.SessionName);
				if (!LobbySettings)
				{
					return;
				}
				LobbySettings->Set(FName("setting_update_action_type"), FString("SearchMatchmakingAction"), EOnlineDataAdvertisementType::ViaOnlineService);
				SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);*/

				SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnJoinMatchMakingCompleted);
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server found"));

				FString SessionName;
				const FOnlineSessionSettings SessionSettings = SessionSearch->SearchResults[0].Session.SessionSettings;
				SessionSettings.Get(FName(SESSION_SETTINGS_SESSION_NAME), SessionName);

				SessionPtr->JoinSession(0, FName(SessionName), SessionSearch->SearchResults[0]);
				SessionSearch.Reset();
			}
			else
			{
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server not found!"));
				if(m_FindMatchmakingAttempt < m_MaxFindMatchmakingAttempt)
				{
					m_FindMatchmakingAttempt++;
					FindMatchMaking();
				}else
				{
					//TODO: send event error
				}
			}
		}
	}
}

bool UEOSSessionSubsystem::JoinMatchMaking(FName SessionName, const FOnlineSessionSearchResult& OnlineSessionSearchResult)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Connexion in progress...!"));
	if (!OnlineSessionSearchResult.IsValid())
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("SessionSearch is not valid!"));
		return false;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
	{
		if (IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			OnMatchMakingJoiningDelegate.Broadcast();

			SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnJoinMatchMakingCompleted);
			UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server found"));

			/*FString SessionName;
			const FOnlineSessionSettings SessionSettings = SessionSearch->SearchResults[0].Session.SessionSettings;
			SessionSettings.Get(FName(SESSION_SETTINGS_SESSION_NAME), SessionName);*/

			if (SessionPtr->JoinSession(0, FName(SessionName), OnlineSessionSearchResult))
			{
				SessionSearch.Reset();
				return true;
			};
		}
	}

	return false;
}

void UEOSSessionSubsystem::OnJoinMatchMakingCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Session %s joined callback"), *SessionName.ToString());
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), 0))
		{
			if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
			{
				if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
				{
					Request request = GetResolvedAddress<DynamicResolverType>::invoke(SessionPtr, SessionName);
					UE_LOG(ModuleNetworkEOS, Warning, TEXT("Join adresse is %s"), *request.getAddress());
					
					if (request.isValid())
					{
						UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetOuterUGameInstance());
						if (!GameInstance)
						{
							UE_LOG(ModuleNetworkEOS, Warning, TEXT("GameInstance null"));
							return;
						}
						GameInstance->NetworkData.SessionName = SessionName;
						GameInstance->NetworkData.SessionState = ESessionState::Joined;

						SessionInfo.SessionName = SessionName;
						WARNING_LOG(TEXT("SESSION NAME TO JOIN : %s"), *SessionInfo.SessionName.ToString());
						SessionInfo.bValid = true;
						SessionInfo.bUseEOSRegister = true;

						if (UCacheSubsystem* Cache = UGameInstance::GetSubsystem<UCacheSubsystem>(GetGameInstance())) {
							Cache->Set(C_SESSION_INFO, SessionInfo);
						}

						UE_LOG(ModuleNetworkEOS, Warning, TEXT("Travel player"));
						PlayerController->ClientTravel(request.getUrl(), ETravelType::TRAVEL_Absolute);
					}
					SessionPtr->ClearOnJoinSessionCompleteDelegates(this);
				}
			}
		}
	}
}

bool UEOSSessionSubsystem::FindMatchMakingWithLobby()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
	{
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			UE_LOG(ModuleNetworkEOS, Warning, TEXT("Search With Lobby in progress...!"));

			//LobbyGameState->OnRep_NetMulticast_FindMatchMakingWithLobby();

			/*FLobbyInfo LobbyInfo;
			bool bIsLobbyCached = CacheSubsystem->Get(UEOSLobbySubsystem::CACHE_LOBBY_INFO, LobbyInfo);
			if (!bIsLobbyCached)
			{
				return false;
			}*/

			UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetGameInstance());
			if (!LobbySubsystem)
			{
				return false;
			}

			FLobbyInfo LobbyInfo = LobbySubsystem->GetLobbyInfo();

			FOnlineSessionSettings* LobbySettings = SessionPtr->GetSessionSettings(FName(LobbyInfo.SessionName));
			if (!LobbySettings)
			{
				return false;
			}
			LobbySettings->Set(FName("setting_update_action_type"), FString("SearchMatchmakingAction"), EOnlineDataAdvertisementType::ViaOnlineService);
			SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);

			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->TimeoutInSeconds = 60.f;
			SessionSearch->QuerySettings.Set(SEARCH_MINSLOTSAVAILABLE, 1, EOnlineComparisonOp::GreaterThanEquals);

			//TODO: ajouter une contrainte pour rechercher uniquement les serveurs dédier
			SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnFindMatchMakingWithLobby);
			return SessionPtr->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}
	return false;
}

void UEOSSessionSubsystem::OnFindMatchMakingWithLobby(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
				SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
			}
		}
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Number of session finded : %i"), SessionSearch.Get()->SearchResults.Num());

		if (SessionSearch.Get()->SearchResults.Num() <= 0)
		{
			FindMatchMakingWithLobby();
		}
		else
		{
			JoinMatchMakingWithLobby();
		}
	}
}

void UEOSSessionSubsystem::JoinMatchMakingWithLobby()
{

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Connexion With Lobby in progress...!"));
	if (!SessionSearch.IsValid())
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("SessionSearch is not valid!"));
		return;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
	{
		if (IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
		{
			if (SessionSearch.Get()->SearchResults.Num() > 0)
			{
				m_FindMatchmakingAttempt = 0;
				OnMatchMakingJoiningDelegate.Broadcast();

				SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnJoinMatchMakingWithLobbyComplete);
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server found"));

				/*FLobbyInfo LobbyInfo;
				bool bIsLobbyCached = CacheSubsystem->Get(UEOSLobbySubsystem::CACHE_LOBBY_INFO, LobbyInfo);
				if (!bIsLobbyCached)
				{
					return;
				}*/

				UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetGameInstance());
				if (!LobbySubsystem)
				{
					return;
				}

				FLobbyInfo LobbyInfo = LobbySubsystem->GetLobbyInfo();

				FOnlineSessionSettings* LobbySettings = SessionPtr->GetSessionSettings(LobbyInfo.SessionName);
				LobbySettings->Set(FName("setting_update_action_type"), FString("JoinMatchmakingAction"), EOnlineDataAdvertisementType::ViaOnlineService);
				SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);

				FString SessionName;
				const FOnlineSessionSettings SessionSettings = SessionSearch->SearchResults[0].Session.SessionSettings;
				SessionSettings.Get(FName(SESSION_SETTINGS_SESSION_NAME), SessionName);

				WARNING_LOG(TEXT("SESSION FOUND NAME %s"), *SessionName)
				if (SessionPtr->JoinSession(0, FName(SessionName), SessionSearch->SearchResults[0]))
				{
					SessionSearch.Reset();
				}
			}
		}
	}
}

void UEOSSessionSubsystem::OnJoinMatchMakingWithLobbyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Session %s joined callback"), *SessionName.ToString());
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld()))
		{
			if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface())
			{
				Request request = GetResolvedAddress<DynamicResolverType>::invoke(SessionPtr, SessionName);
				FString SessionId = GetSessionId::invoke(SessionPtr, SessionName);
				UE_LOG(ModuleNetworkEOS, Warning, TEXT("Join adresse is %s"), *request.getAddress());

				if (request.isValid())
				{
					/*if (!LobbyGameState)
					{
						request.addParams(R_PARAMS_LOBBY_ID, LobbyGameState->GetLobbyId());
					}*/

					UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetOuterUGameInstance());
					if (!GameInstance)
					{
						UE_LOG(ModuleNetworkEOS, Warning, TEXT("GameInstance null"));
						return;
					}
					GameInstance->NetworkData.SessionName = SessionName;
					GameInstance->NetworkData.SessionState = ESessionState::Joined;

					/*SessionInfo.SessionName = SessionName;
					SessionInfo.bValid = true;
					SessionInfo.bUseEOSRegister = true;*/

					UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetGameInstance());
					if (!LobbySubsystem)
					{
						return;
					}

					FLobbyInfo LobbyInfo = LobbySubsystem->GetLobbyInfo();
					if (!LobbyInfo.bValid)
					{
						return;
					}

			/*		if (!CacheSubsystem)
					{
						return;
					}*/

					/*FLobbyInfo LobbyInfo;
					bool bIsLobbyCached = CacheSubsystem->Get(UEOSLobbySubsystem::CACHE_LOBBY_INFO, LobbyInfo);
					if (!bIsLobbyCached)
					{
						return;
					}*/

					FOnlineSessionSettings* LobbySettings = SessionPtr->GetSessionSettings(LobbyInfo.SessionName);
					LobbySettings->Set(FName("setting_update_action_type"), FString("TravelMatchmakingAction"), EOnlineDataAdvertisementType::ViaOnlineService);
					LobbySettings->Set(FName("server_address"), request.getAddress(), EOnlineDataAdvertisementType::ViaOnlineService);
					LobbySettings->Set(FName("server_params"), request.getParamsToString(), EOnlineDataAdvertisementType::ViaOnlineService);
					LobbySettings->Set(FName("session_name"), SessionName.ToString());
					SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);

					SessionInfo.SessionName = SessionName;
					SessionInfo.bValid = true;
					SessionInfo.bUseEOSRegister = true;

					if (UCacheSubsystem* Cache = UGameInstance::GetSubsystem<UCacheSubsystem>(GetGameInstance())) {
						Cache->Set(C_SESSION_INFO, SessionInfo);
					}
				/*	CacheSubsystem->Set(UEOSSessionSubsystem::C_SESSION_INFO, SessionInfo);*/

					if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), 0))
					{
						if (request.isValid())
						{
							PlayerController->ClientTravel(request.getUrl(), ETravelType::TRAVEL_Absolute);
						}
					}

					/*FOnlineSessionSettings* SearchSettings = SessionPtr->GetSessionSettings(FName("Dev_Lobby"));
					if (SearchSettings)
					{
						SearchSettings->Set(FName("TEST"), FName("UPDATED"));
					}
					SessionPtr->UpdateSession(FName("Dev_Lobby"), *SearchSettings);*/

				/*	const FRequestDTO RequestDto = Request::makeDTO(request);
					FSessionInfo SessionInfo;
					SessionInfo.SessionId = SessionId;
					SessionInfo.SessionName = SessionName;
					SessionInfo.RequestDto = RequestDto;

					FEOSSessionSearchResult SessionSearchResultInfo;
					SessionSearchResultInfo.SessionSearchResult = SessionSearch->SearchResults[0];
					SessionSearchResultInfo.SessionInfo = SessionInfo;*/

					//CacheSubsystem->Set(FString("MatchMakingInfo"), SessionSearchResultInfo);

					//LobbyGameState->OnRep_NetMutlicast_JoinMatchMakinWithLobby(SessionInfo);
				}
				SessionPtr->ClearOnJoinSessionCompleteDelegates(this);
			}
		}
	}
}

void UEOSSessionSubsystem::MatchMaking()
{
	/*FLobbyInfo LobbyInfo;
	bool LobbyInfoCached = false;
	if (CacheSubsystem)
	{
		LobbyInfoCached = CacheSubsystem->Get(FString("C_LOBBY_INFO"), LobbyInfo);
	}*/

	UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetGameInstance());
	if (!LobbySubsystem)
	{
		FindMatchMaking();
	}

	FLobbyInfo LobbyInfo = LobbySubsystem->GetLobbyInfo();

	if (LobbyInfo.bValid)
	{
		FindMatchMakingWithLobby();
	}
	else
	{
		FindMatchMaking();
	}
}

void UEOSSessionSubsystem::QuitSession(FName SessionName)
{
	WARNING_LOG(TEXT("SESSION NAME TO JOIN : %s"), *SessionInfo.SessionName.ToString());

	if (SessionInfo.bValid && SessionInfo.bUseEOSRegister)
	{
		DestroySession(SessionInfo.SessionName);
	}
	else
	{
		if (const UModuleNetworkEOSSettings* ConnectSetting = Cast<UModuleNetworkEOSSettings>(UModuleNetworkEOSSettings::StaticClass()->GetDefaultObject(false)))
		{
			UGameplayStatics::OpenLevel(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), FName(ConnectSetting->DefaultMenuMapText.GetAssetName()));
		}
	}
}

void UEOSSessionSubsystem::UpdateSessionStatus(const FString& Status, const FName& SessionName)
{
	
}

void UEOSSessionSubsystem::EndSession()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
			SessionPtr->EndSession(SessionInfo.SessionName);
			SessionInfo.bRestartEnd = true;
		}
	}
}

void UEOSSessionSubsystem::StartSession()
{
	if (!SessionInfo.bValid || !SessionInfo.bRestartEnd) {
		return;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
			SessionInfo.bRestartEnd = false;
			SessionPtr->StartSession(SessionInfo.SessionName);
		}
	}
}

FSessionInfo UEOSSessionSubsystem::GetSessionInfo()
{
	return SessionInfo;
}

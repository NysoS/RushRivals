// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Settings/ModuleNetworkEOSSettings.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
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

	if (!EOSOnlineSubsytem)
		return;

	m_SessionPtr = EOSOnlineSubsytem->GetSessionInterface();
	if (!m_SessionPtr)
	{
		ERROR_LOG(TEXT("Session Interface cannot be instanciate"));
	}
	WARNING_LOG(TEXT("Session Subsystem initialized"));


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
	if (!m_SessionPtr)
		return;

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

	m_SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnCreateSessionProgress);
	m_SessionPtr->CreateSession(0, FName("Dev_Session"), SessionSettings);
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

		FOnlineSessionSettings* SessionSettings = m_SessionPtr->GetSessionSettings(SessionName);
		if (SessionSettings)
		{
			SessionSettings->Set(FName(SESSION_SETTINGS_PORT), FString::FromInt(GetWorld()->URL.Port), EOnlineDataAdvertisementType::ViaOnlineService);
			m_SessionPtr->UpdateSession(SessionName, *SessionSettings);
		}

		m_SessionPtr->ClearOnCreateSessionCompleteDelegates(this);

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
	if (!m_SessionPtr)
		return;

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

	m_SessionPtr->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionServerCompleteDelegates);
	m_SessionPtr->CreateSession(0, SessionName, SessionSettings);
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

		FOnlineSessionSettings* SessionSettings = m_SessionPtr->GetSessionSettings(SessionName);
		if (SessionSettings)
		{
			SessionSettings->Set(FName(SESSION_SETTINGS_PORT), FString::FromInt(GetWorld()->URL.Port), EOnlineDataAdvertisementType::ViaOnlineService);
			m_SessionPtr->UpdateSession(SessionName, *SessionSettings);
		}

		m_SessionPtr->ClearOnCreateSessionCompleteDelegates(this);

		OnCreateSessionComplete.Broadcast(SessionName, bWasSuccessful);
	}
}

bool UEOSSessionSubsystem::DestroySession(FName SessionName)
{
	if (!m_SessionPtr)
		return false;

	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	OnDestroySessionCompleteDelegate.BindUObject(this, &UEOSSessionSubsystem::OnDestroySessionProgress);

	m_SessionPtr->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
	return m_SessionPtr->DestroySession(SessionName, OnDestroySessionCompleteDelegate);
}

bool UEOSSessionSubsystem::DestroySessionAllClient(FName SessionName)
{
	if (!m_SessionPtr)
		return false;

	UEOSGameInstance* GameInstance = Cast<UEOSGameInstance>(GetOuterUGameInstance());
	if (!GameInstance)
	{
		return false;
	}

	return m_SessionPtr->DestroySession(GameInstance->NetworkData.SessionName);
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
	}
}

bool UEOSSessionSubsystem::FindSession()
{
	if (!m_SessionPtr)
		return false;

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Search in progress...!"));

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->QuerySettings.SearchParams.Empty();

	m_SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnFindSessionCompleted);
	return m_SessionPtr->FindSessions(0, SessionSearch.ToSharedRef());
}

void UEOSSessionSubsystem::OnFindSessionCompleted(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		m_SessionPtr->ClearOnFindSessionsCompleteDelegates(this);

		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Number of session finded : %i"), SessionSearch.Get()->SearchResults.Num());
	}
}

void UEOSSessionSubsystem::JoinSession()
{
	if (!m_SessionPtr)
		return;

	if (!SessionSearch.IsValid())
		return;

	if (SessionSearch.Get()->SearchResults.Num() > 0)
	{
		m_SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnJoinSessionCompleted);
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server found"));

		m_SessionPtr->JoinSession(0, FName("Dev_Session"), SessionSearch->SearchResults[0]);
		SessionSearch.Reset();
	}
	else
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server not found!"));
	}
}

void UEOSSessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Session %s joined callback"), *SessionName.ToString());
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), 0))
		{
			FString JoinAddress;
			m_SessionPtr->GetResolvedConnectString(SessionName, JoinAddress);

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
			m_SessionPtr->ClearOnJoinSessionCompleteDelegates(this);
		}
	}
}

bool UEOSSessionSubsystem::FindMatchMaking()
{
	if (!m_SessionPtr)
		return false;

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Search in progress...!"));

	OnMatchMakingProgressDelegate.Broadcast();

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->TimeoutInSeconds = 60.f;
	SessionSearch->QuerySettings.Set(SEARCH_MINSLOTSAVAILABLE, 1, EOnlineComparisonOp::GreaterThanEquals);

	//TODO: ajouter une contrainte pour rechercher uniquement les serveurs dédier
	m_SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnFindMatchMakingCompleted);
	return m_SessionPtr->FindSessions(0, SessionSearch.ToSharedRef());
}

void UEOSSessionSubsystem::OnFindMatchMakingCompleted(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		m_SessionPtr->ClearOnFindSessionsCompleteDelegates(this);

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
	if (!m_SessionPtr)
		return;

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Connexion in progress...!"));
	if (!SessionSearch.IsValid())
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("SessionSearch is not valid!"));
		return;
	}

	if (SessionSearch.Get()->SearchResults.Num() > 0)
	{
		m_FindMatchmakingAttempt = 0;
		OnMatchMakingJoiningDelegate.Broadcast();

		m_SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnJoinMatchMakingCompleted);
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server found"));

		FString SessionName;
		const FOnlineSessionSettings SessionSettings = SessionSearch->SearchResults[0].Session.SessionSettings;
		SessionSettings.Get(FName(SESSION_SETTINGS_SESSION_NAME), SessionName);

		m_SessionPtr->JoinSession(0, FName(SessionName), SessionSearch->SearchResults[0]);
		SessionSearch.Reset();
	}
	else
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server not found!"));
		if (m_FindMatchmakingAttempt < m_MaxFindMatchmakingAttempt)
		{
			m_FindMatchmakingAttempt++;
			FindMatchMaking();
		}
		else
		{
			//TODO: send event error
		}
	}
}

bool UEOSSessionSubsystem::JoinMatchMaking(FName SessionName, const FOnlineSessionSearchResult& OnlineSessionSearchResult)
{
	if (!m_SessionPtr)
		return false;

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Connexion in progress...!"));
	if (!OnlineSessionSearchResult.IsValid())
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("SessionSearch is not valid!"));
		return false;
	}

	OnMatchMakingJoiningDelegate.Broadcast();

	m_SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnJoinMatchMakingCompleted);
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server found"));

	if (m_SessionPtr->JoinSession(0, FName(SessionName), OnlineSessionSearchResult))
	{
		SessionSearch.Reset();
		return true;
	};

	return false;
}

void UEOSSessionSubsystem::OnJoinMatchMakingCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Session %s joined callback"), *SessionName.ToString());
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), 0))
		{
			Request request = GetResolvedAddress<DynamicResolverType>::invoke(m_SessionPtr, SessionName);
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
			m_SessionPtr->ClearOnJoinSessionCompleteDelegates(this);
		}
	}
}

bool UEOSSessionSubsystem::FindMatchMakingWithLobby()
{
	if (!m_SessionPtr)
		return false;

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Search With Lobby in progress...!"));

	UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetGameInstance());
	if (!LobbySubsystem)
	{
		return false;
	}

	FLobbyInfo LobbyInfo = LobbySubsystem->GetLobbyInfo();

	FOnlineSessionSettings* LobbySettings = m_SessionPtr->GetSessionSettings(FName(LobbyInfo.SessionName));
	if (!LobbySettings)
	{
		return false;
	}
	LobbySettings->Set(FName("setting_update_action_type"), FString("SearchMatchmakingAction"), EOnlineDataAdvertisementType::ViaOnlineService);
	m_SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->TimeoutInSeconds = 60.f;
	SessionSearch->QuerySettings.Set(SEARCH_MINSLOTSAVAILABLE, 1, EOnlineComparisonOp::GreaterThanEquals);

	//TODO: ajouter une contrainte pour rechercher uniquement les serveurs dédier
	m_SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnFindMatchMakingWithLobby);
	return m_SessionPtr->FindSessions(0, SessionSearch.ToSharedRef());
}

void UEOSSessionSubsystem::OnFindMatchMakingWithLobby(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		m_SessionPtr->ClearOnFindSessionsCompleteDelegates(this);

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
	if (!m_SessionPtr)
		return;

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Connexion With Lobby in progress...!"));
	if (!SessionSearch.IsValid())
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("SessionSearch is not valid!"));
		return;
	}

	if (SessionSearch.Get()->SearchResults.Num() > 0)
	{
		m_FindMatchmakingAttempt = 0;
		OnMatchMakingJoiningDelegate.Broadcast();

		m_SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UEOSSessionSubsystem::OnJoinMatchMakingWithLobbyComplete);
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server found"));

		UEOSLobbySubsystem* LobbySubsystem = UGameInstance::GetSubsystem<UEOSLobbySubsystem>(GetGameInstance());
		if (!LobbySubsystem)
		{
			return;
		}

		FLobbyInfo LobbyInfo = LobbySubsystem->GetLobbyInfo();

		FOnlineSessionSettings* LobbySettings = m_SessionPtr->GetSessionSettings(LobbyInfo.SessionName);
		LobbySettings->Set(FName("setting_update_action_type"), FString("JoinMatchmakingAction"), EOnlineDataAdvertisementType::ViaOnlineService);
		m_SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);

		FString SessionName;
		const FOnlineSessionSettings SessionSettings = SessionSearch->SearchResults[0].Session.SessionSettings;
		SessionSettings.Get(FName(SESSION_SETTINGS_SESSION_NAME), SessionName);

		WARNING_LOG(TEXT("SESSION FOUND NAME %s"), *SessionName)
		if (m_SessionPtr->JoinSession(0, FName(SessionName), SessionSearch->SearchResults[0]))
		{
			SessionSearch.Reset();
		}
	}
}

void UEOSSessionSubsystem::OnJoinMatchMakingWithLobbyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Session %s joined callback"), *SessionName.ToString());
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		Request request = GetResolvedAddress<DynamicResolverType>::invoke(m_SessionPtr, SessionName);
		FString SessionId = GetSessionId::invoke(m_SessionPtr, SessionName);
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

			FOnlineSessionSettings* LobbySettings = m_SessionPtr->GetSessionSettings(LobbyInfo.SessionName);
			LobbySettings->Set(FName("setting_update_action_type"), FString("TravelMatchmakingAction"), EOnlineDataAdvertisementType::ViaOnlineService);
			LobbySettings->Set(FName("server_address"), request.getAddress(), EOnlineDataAdvertisementType::ViaOnlineService);
			LobbySettings->Set(FName("server_params"), request.getParamsToString(), EOnlineDataAdvertisementType::ViaOnlineService);
			LobbySettings->Set(FName("session_name"), SessionName.ToString());
			m_SessionPtr->UpdateSession(LobbyInfo.SessionName, *LobbySettings);

			SessionInfo.SessionName = SessionName;
			SessionInfo.bValid = true;
			SessionInfo.bUseEOSRegister = true;

			if (UCacheSubsystem* Cache = UGameInstance::GetSubsystem<UCacheSubsystem>(GetGameInstance())) {
				Cache->Set(C_SESSION_INFO, SessionInfo);
			}

			if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(UNetworkEOSBPFunctionLibrary::GetActiveWorld(), 0))
			{
				if (request.isValid())
				{
					PlayerController->ClientTravel(request.getUrl(), ETravelType::TRAVEL_Absolute);
				}
			}
		}
		m_SessionPtr->ClearOnJoinSessionCompleteDelegates(this);
	}
}

void UEOSSessionSubsystem::MatchMaking()
{
	if (!m_SessionPtr)
		return;

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
	if (!m_SessionPtr)
		return;

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
	if (!m_SessionPtr)
		return;

	m_SessionPtr->EndSession(SessionInfo.SessionName);
	SessionInfo.bRestartEnd = true;
}

void UEOSSessionSubsystem::StartSession()
{
	if (!m_SessionPtr)
		return;
	
	if (!SessionInfo.bValid || !SessionInfo.bRestartEnd) {
		return;
	}

	SessionInfo.bRestartEnd = false;
	m_SessionPtr->StartSession(SessionInfo.SessionName);
}

FSessionInfo UEOSSessionSubsystem::GetSessionInfo()
{
	return SessionInfo;
}

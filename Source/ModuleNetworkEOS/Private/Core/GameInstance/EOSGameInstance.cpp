// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameInstance/EOSGameInstance.h"

#include "ModuleNetworkEOSModules.h"
#include "MoviePlayer.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "UObject/Object.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "Core/Subsystem/EOSConnectSubsystem.h"
#include "Core/Subsystem/EOSFriendSubsystem.h"
#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Server/DedicateServerInstance.h"

UEOSGameInstance::UEOSGameInstance()
{
	m_ServerInstance = NewObject<UDedicateServerInstance>();
}

void UEOSGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Warning, TEXT("[UEOS_GameInstance] : Init"));
	GetEngine()->OnNetworkFailure().AddUObject(this, &UEOSGameInstance::HandleNetworkFailureDele);

	if (m_ServerInstance)
	{
		m_ServerInstance->InitInstance(this);
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("ServerInstance name : %s"), *m_ServerInstance->GetServerInstanceName().ToString());
	}

#if UE_SERVER == 1
	if (m_ServerInstance)
	{
		m_ServerInstance->CreateInstance();
	}
#else
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEOSGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UEOSGameInstance::EndLoadingScreen);

	FTimerHandle HandleToLogin;
	GetWorld()->GetTimerManager().SetTimer(HandleToLogin, [&]()->void
	{
		AutoLogin();
	}, 5, false);
#endif
}

void UEOSGameInstance::BeginLoadingScreen(const FString& MapName)
{
	if (IsDedicatedServerInstance())
	{
		return;
	}

	WARNING_LOG(TEXT("BeginLoading"))
	
	const UModuleNetworkEOSSettings* ConnectSetting = Cast<UModuleNetworkEOSSettings>(UModuleNetworkEOSSettings::StaticClass()->GetDefaultObject(false));
	if (!ConnectSetting)
	{
		return;
	}

	//auto LWC = LoadClass<UWidget>(this, TEXT("WBP_LoadingScreen"), TEXT("/Game/HitBox/Widget/WBP_LoadingScreen"));
	auto LoadingWidget = CreateWidget<UUserWidget>(this, ConnectSetting->LoadingScreenClass, TEXT("LoadingScreen"));

	FLoadingScreenAttributes LoadingScreenAttributes;
	LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreenAttributes.WidgetLoadingScreen = LoadingWidget == nullptr ? FLoadingScreenAttributes::NewTestLoadingScreenWidget() : LoadingWidget->TakeWidget();
	LoadingScreenAttributes.bWaitForManualStop = false;

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
}

void UEOSGameInstance::EndLoadingScreen(UWorld* World)
{
}

void UEOSGameInstance::BeginDestroy()
{
	Super::BeginDestroy();

	UEOSSessionSubsystem* SessionSubsystem = GetSubsystem<UEOSSessionSubsystem>();
	if (!SessionSubsystem)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Impossible to get Session Subsystem"));
		return;
	}

	if (SessionSubsystem->DestroySessionAllClient(NetworkData.SessionName))
	{
		NetworkData.SessionName = FName("");
	}
}

void UEOSGameInstance::HandleNetworkFailureDele(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	if(FailureType == ENetworkFailure::ConnectionLost || FailureType == ENetworkFailure::ConnectionTimeout)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("HandleNetworkFailure ConnectionLost"));

		UEOSSessionSubsystem* SessionSubsystem = GetSubsystem<UEOSSessionSubsystem>();
		if(!SessionSubsystem)
		{
			UE_LOG(ModuleNetworkEOS, Warning, TEXT("Impossible to get Session Subsystem"));
			return;
		}

		if(SessionSubsystem->DestroySessionAllClient(NetworkData.SessionName))
		{
			NetworkData.SessionName = FName("");
		}
	}
}

void UEOSGameInstance::AutoLogin()
{
	const UModuleNetworkEOSSettings* ConnectSetting = Cast<UModuleNetworkEOSSettings>(UModuleNetworkEOSSettings::StaticClass()->GetDefaultObject(false));
	if (!ConnectSetting)
	{
		return;
	}

	UEOSConnectSubsystem* ConnectInterface = GetSubsystem<UEOSConnectSubsystem>();
	if (!ConnectInterface)
		return;


	if (ConnectSetting && ConnectInterface) {

#if WITH_EDITOR
		bool LoginSuccessfull = ConnectSetting->bUseAccountPortal ? ConnectInterface->Login() : ConnectInterface->LoginWithDevAuth(ConnectSetting->IpSdk + ":" + ConnectSetting->PortSdk, ConnectSetting->DefaultUser);
#else
		bool LoginSuccessfull = ConnectInterface->LoginWithLuncher();
#endif

		if (!LoginSuccessfull)
		{
			UE_LOG(ModuleNetworkEOS, Warning, TEXT("Login failed"));
		}
	}
}

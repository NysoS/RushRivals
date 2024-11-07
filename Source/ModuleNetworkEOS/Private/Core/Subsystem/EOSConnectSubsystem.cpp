// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystem/EOSConnectSubsystem.h"

#include "ModuleNetworkEOSModules.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"

void UEOSConnectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

bool UEOSConnectSubsystem::Login()
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld()))
	{
		if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
		{
				FOnlineAccountCredentials AccountCredentials;
				AccountCredentials.Id = "";
				AccountCredentials.Token = "";
				AccountCredentials.Type = "accountportal";

				IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UEOSConnectSubsystem::OnLoginProgress);

				return IdentityPtr->Login(0, AccountCredentials);
		}
	}

	return false;
}

bool UEOSConnectSubsystem::LoginWithDevAuth(const FString& Id, const FString& Token)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("[UEOS_DevAuth_Connect] : Login : Token %s, ID %s"), *Token, *Id);
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld()))
	{
		if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
		{
			FOnlineAccountCredentials AccountCredentials;
			AccountCredentials.Id = Id;
			AccountCredentials.Token = Token;
			AccountCredentials.Type = "Developer";

			OnLoginProgressDelegate.Broadcast();

			IdentityPtr->OnLoginStatusChangedDelegates->AddUObject(this, &UEOSConnectSubsystem::OnConnectionStatusChanged);
			IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UEOSConnectSubsystem::OnLoginProgress);
			return IdentityPtr->Login(0, AccountCredentials);
		}
	}

	return false;
}

bool UEOSConnectSubsystem::LoginWithLuncher()
{
	WARNING_LOG(TEXT("LOGIN WITH LAUNCHER"))

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld()))
	{
		if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
		{
			
			OnLoginProgressDelegate.Broadcast();

			FString Type, Token;
			FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), Type);
			FParse::Value(FCommandLine::Get(), TEXT("AUTH_PASSWORD="), Token);

			FString EpicApp;
			FParse::Value(FCommandLine::Get(), TEXT("epicapp="), EpicApp);


			WARNING_LOG(TEXT("AUTH_PASSWORD %s"), *Token)
			WARNING_LOG(TEXT("AUTH TYPE %s"), *Type);
			WARNING_LOG(TEXT("Epic App %s"), *EpicApp);

			IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UEOSConnectSubsystem::OnAutoLoginComplete);
			if (!IdentityPtr->AutoLogin(0))
			{
				Login();
			}


			return true;
		}
	}

	return false;
}

void UEOSConnectSubsystem::OnAutoLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
	const FString& Error)
{
	if (bWasSuccessful)
	{
		OnLoginComplete.Broadcast(bWasSuccessful);
		UE_LOG(LogTemp, Log, TEXT("Autologin réussi pour l'utilisateur %s"), *UserId.ToString());

		if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld()))
		{
			if (const IOnlineIdentityPtr IdentityPtr = Subsystem->GetIdentityInterface())
			{
				WARNING_LOG(TEXT("NICKNAME %s"), *IdentityPtr->GetPlayerNickname(UserId));

				auto t = IdentityPtr->GetUserAccount(UserId);
				WARNING_LOG(TEXT("User Account %s"), *t.Get()->GetDisplayName());
				
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Autologin échoué : %s"), *Error);
	}
}

void UEOSConnectSubsystem::OnLoginProgress(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
                                           const FString& Error)
{
	OnLoginComplete.Broadcast(bWasSuccessful);
	if (bWasSuccessful)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("[UEOS_Connect] : Login Successful"));
	}
	else
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("[UEOS_Connect] : Login fail reson - %S"), *Error);
	}
}

void UEOSConnectSubsystem::Logout(int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld()))
	{
		if (const IOnlineIdentityPtr IdentityPrt = Subsystem->GetIdentityInterface())
		{
			IdentityPrt->OnLogoutCompleteDelegates->AddUObject(this, &UEOSConnectSubsystem::OnLogoutProgress);
			IdentityPrt->Logout(LocalUserNum);
		}
	}
}

void UEOSConnectSubsystem::OnLogoutProgress(int32 LocalUserNum, bool bWasSuccessful)
{
	OnLogoutComplete.Broadcast(LocalUserNum, bWasSuccessful);
	if (bWasSuccessful)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("[UEOS_Connect] : Logout Successful"));
	}
}

void UEOSConnectSubsystem::OnConnectionStatusChanged(int32 LocalUserNum, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId)
{
	if(NewStatus != ELoginStatus::LoggedIn)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Connection Status! Error type : %d"), NewStatus);
	}
}

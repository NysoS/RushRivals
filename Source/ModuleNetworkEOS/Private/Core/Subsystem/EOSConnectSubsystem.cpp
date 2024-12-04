// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystem/EOSConnectSubsystem.h"

#include "ModuleNetworkEOSModules.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlinePresenceInterface.h"

void UEOSConnectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	m_IdentityPtr = EOSOnlineSubsytem->GetIdentityInterface();

	WARNING_LOG(TEXT("Connect Subsystem initialized"));
}

void UEOSConnectSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UEOSConnectSubsystem::Login()
{
	if (!m_IdentityPtr)
	{
		return false;
	}

	FOnlineAccountCredentials AccountCredentials;
	AccountCredentials.Id = "";
	AccountCredentials.Token = "";
	AccountCredentials.Type = "accountportal";

	m_IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UEOSConnectSubsystem::OnLoginProgress);

	return m_IdentityPtr->Login(0, AccountCredentials);
}

bool UEOSConnectSubsystem::LoginWithDevAuth(const FString& Id, const FString& Token)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("[UEOS_DevAuth_Connect] : Login : Token %s, ID %s"), *Token, *Id);

	if (!m_IdentityPtr)
	{
		return false;
	}

	FOnlineAccountCredentials AccountCredentials;
	AccountCredentials.Id = Id;
	AccountCredentials.Token = Token;
	AccountCredentials.Type = "Developer";

	OnLoginProgressDelegate.Broadcast();

	m_IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UEOSConnectSubsystem::OnLoginProgress);
	
	return m_IdentityPtr->Login(0, AccountCredentials);
}

bool UEOSConnectSubsystem::LoginWithLuncher()
{
	WARNING_LOG(TEXT("LOGIN WITH LAUNCHER"));

	if (!m_IdentityPtr)
	{
		return false;
	}

	OnLoginProgressDelegate.Broadcast();

	FString Type, Token;
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), Type);
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_PASSWORD="), Token);

	FString EpicApp;
	FParse::Value(FCommandLine::Get(), TEXT("epicapp="), EpicApp);

	WARNING_LOG(TEXT("AUTH_PASSWORD %s"), *Token);
	WARNING_LOG(TEXT("AUTH TYPE %s"), *Type);
	WARNING_LOG(TEXT("Epic App %s"), *EpicApp);

	m_IdentityPtr->OnLoginCompleteDelegates->AddUObject(this, &UEOSConnectSubsystem::OnAutoLoginComplete);
	if (!m_IdentityPtr->AutoLogin(0))
	{
		Login();
	}

	return true;
}

void UEOSConnectSubsystem::OnAutoLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
	const FString& Error)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Autologin échoué : %s"), *Error);
		return;
	}

	OnLoginComplete.Broadcast(bWasSuccessful);
	UE_LOG(LogTemp, Log, TEXT("Autologin réussi pour l'utilisateur %s"), *UserId.ToString());

	if (!m_IdentityPtr)
	{
		return;
	}

	WARNING_LOG(TEXT("NICKNAME %s"), *m_IdentityPtr->GetPlayerNickname(UserId));

	auto t = m_IdentityPtr->GetUserAccount(UserId);
	WARNING_LOG(TEXT("User Account %s"), *t.Get()->GetDisplayName());
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
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("[UEOS_Connect] : Login fail reson - %s"), *Error);
	}
}

void UEOSConnectSubsystem::Logout(int32 LocalUserNum)
{
	if (!m_IdentityPtr)
	{
		return;
	}

	m_IdentityPtr->OnLogoutCompleteDelegates->AddUObject(this, &UEOSConnectSubsystem::OnLogoutProgress);
	m_IdentityPtr->Logout(LocalUserNum);
}

void UEOSConnectSubsystem::OnLogoutProgress(int32 LocalUserNum, bool bWasSuccessful)
{
	OnLogoutComplete.Broadcast(LocalUserNum, bWasSuccessful);
	if (bWasSuccessful)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("[UEOS_Connect] : Logout Successful"));
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystem/EOSUserSubsystem.h"

#include "Interfaces/OnlineIdentityInterface.h"

void UEOSUserSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	//m_IdentityPtr = EOSOnlineSubsytem->GetIdentityInterface();
}

void UEOSUserSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UEOSUserSubsystem::GetPlayerStatus() const
{
	if (!m_IdentityPtr)
		return false;

	return m_IdentityPtr->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

FString UEOSUserSubsystem::GetPlayerUsername() const
{
	if (!m_IdentityPtr)
		return FString();

	if (!GetPlayerStatus())
		return FString();

	return m_IdentityPtr->GetPlayerNickname(0);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Server/DedicateServerInstance.h"

#include "ModuleNetworkEOSModules.h"
#include "OnlineSubsystemUtils.h"
#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Settings/ModuleNetworkEOSSettings.h"

class IOnlineSubsystem;

void UDedicateServerInstance::InitInstance(UGameInstance* GameInstance)
{
	Super::InitInstance(GameInstance);

	if (!m_NetworkSettings)
	{
		return;
	}

	m_ServerProps = FServerInstanceProps{ FName(m_NetworkSettings->GameModeIntanceSelected.ToString()), EServerType::DedicatedServer };
}

void UDedicateServerInstance::CreateInstance()
{
	Super::CreateInstance();

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Create Dedicated Server Instance"));

	FTimerHandle DelayToInitServerSession;

	if(!m_GameInstance)
	{
		UE_LOG(ModuleNetworkEOS, Error, TEXT("No GameInstance founded"));
		return;
	}

	UEOSSessionSubsystem* SessionSubsystem = m_GameInstance->GetSubsystem<UEOSSessionSubsystem>();
	if (!SessionSubsystem)
	{
		UE_LOG(ModuleNetworkEOS, Error, TEXT("Impossible to get Session Subsystem"));
		return;
	}

	const FEOSGameRule* SessionRule = GetRule(m_ServerProps.InstanceName);
	if (!SessionRule)
	{
		return;
	}

	SessionSubsystem->CreateSessionSever(SessionRule->MaxPlayer);
}

void UDedicateServerInstance::ResetInstance()
{
	UEOSSessionSubsystem* SessionSubsystem = m_GameInstance->GetSubsystem<UEOSSessionSubsystem>();
	if (!SessionSubsystem)
	{
		UE_LOG(ModuleNetworkEOS, Error, TEXT("Impossible to get Session Subsystem"));
		return;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
			SessionPtr->EndSession(SessionSubsystem->GetSessionInfo().SessionName);
		}
	}
}

void UDedicateServerInstance::RestartInstance()
{
	UEOSSessionSubsystem* SessionSubsystem = m_GameInstance->GetSubsystem<UEOSSessionSubsystem>();
	if (!SessionSubsystem)
	{
		UE_LOG(ModuleNetworkEOS, Error, TEXT("Impossible to get Session Subsystem"));
		return;
	}

	if (const IOnlineSubsystem* Subsystem = Online::GetSubsystem(UObject::GetWorld())) {
		if (const IOnlineSessionPtr SessionPtr = Subsystem->GetSessionInterface()) {
			SessionPtr->StartSession(SessionSubsystem->GetSessionInfo().SessionName);
		}
	}
}

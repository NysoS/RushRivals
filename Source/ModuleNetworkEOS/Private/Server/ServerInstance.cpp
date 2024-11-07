// Fill out your copyright notice in the Description page of Project Settings.


#include "Server/ServerInstance.h"
#include "ModuleNetworkEOSModules.h"
#include "Settings/ModuleNetworkEOSSettings.h"

void UServerInstance::InitInstance(UGameInstance* GameInstance)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Init Server Instance Base Class"));
	if(!GameInstance)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("No GameInstance"));
		return;
	}

	UModuleNetworkEOSSettings* NetworkSetting = Cast<UModuleNetworkEOSSettings>(UModuleNetworkEOSSettings::StaticClass()->GetDefaultObject(false));
	if(!NetworkSetting)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Network Setting no found or null !"));
		return;
	}

	m_GameInstance = GameInstance;
	m_NetworkSettings = NetworkSetting;
}

void UServerInstance::SwitchMap()
{
}

FName UServerInstance::GetServerInstanceName() const
{
	return m_ServerProps.InstanceName;
}

EServerType UServerInstance::GetServerInstanceType() const
{
	return m_ServerProps.ServerInstanceType;
}

void UServerInstance::CreateInstance()
{
}

FEOSGameRule* UServerInstance::GetRule(FName& key) const
{
	if(!m_NetworkSettings)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Not NetworkSetting found!"));
		return nullptr;
	}

	UDataTable* gameRuleDT = m_NetworkSettings->GameRulesDataTable.LoadSynchronous();
	if(!gameRuleDT)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Not GameRulesDataTable loaded!"));
		return nullptr;
	}

	FString ctx = "";
	return gameRuleDT->FindRow<FEOSGameRule>(key, ctx);
}

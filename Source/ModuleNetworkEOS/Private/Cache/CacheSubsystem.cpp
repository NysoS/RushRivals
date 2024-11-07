// Fill out your copyright notice in the Description page of Project Settings.


#include "Cache/CacheSubsystem.h"

#include "ModuleNetworkEOSModules.h"

void UCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	WARNING_LOG(TEXT("CacheSubsytem init"))
}

void UCacheSubsystem::Deinitialize()
{
	Super::Deinitialize();
	CacheClear();
}

void UCacheSubsystem::Clear(const FString& key)
{
	if (!m_data.Contains(key))
	{
		return;
	}

	m_data.Remove(key);
}

void UCacheSubsystem::CacheClear()
{
	m_data.Reserve(0);
}

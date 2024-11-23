// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Subsystem/EOSGameInstanceSubsystem.h"

#include "ModuleNetworkEOSModules.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystem.h"

void UEOSGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EOSOnlineSubsytem = Online::GetSubsystem(GetWorld());
	if (!EOSOnlineSubsytem)
	{
		ERROR_LOG(TEXT("Online Subsystem cannot be initialized"));
		return;
	}

	ERROR_LOG(TEXT("Online Game Instance Subsystem initialized"));
}

void UEOSGameInstanceSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (EOSOnlineSubsytem != nullptr)
	{
	/*	delete EOSOnlineSubsytem;
		EOSOnlineSubsytem = nullptr;*/
	}
}

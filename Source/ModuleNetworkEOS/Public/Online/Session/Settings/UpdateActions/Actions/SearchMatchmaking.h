#pragma once

#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Online/Session/Settings/UpdateActions/Types/SessionSettingActionType.h"

class UEOSSessionSubsystem;

CREATE_ACTION(SearchMatchmakingAction)
static void execute(const UEOSSessionSubsystem* SessionSubsystem)
{
	SessionSubsystem->OnMatchMakingProgressDelegate.Broadcast();
}
END_ACTION()

CREATE_ACTION(JoinMatchmakingAction)
static void execute(const UEOSSessionSubsystem* SessionSubsystem)
{
	UE_LOG(LogTemp, Warning, TEXT("JoinMatchMaking"));
	SessionSubsystem->OnMatchMakingJoiningDelegate.Broadcast();
}
END_ACTION()

CREATE_ACTION(TravelMatchmakingAction)
static void execute(const Request& request, UWorld* World)
{
	UE_LOG(LogTemp, Warning, TEXT("TravelMatchmakingAction"));

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController)
	{
		return;
	}

	PlayerController->ClientTravel(request.getUrl(), TRAVEL_Absolute);
}
END_ACTION()
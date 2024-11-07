#pragma once

#include "CoreMinimal.h"
#include "NetworkData.generated.h"

UENUM(BlueprintType)
enum ESessionState : int
{
	Session_None,
	Finding,
	Joined,
	In
};

UENUM(BlueprintType)
enum EServerType : int
{
	NoServer,
	DedicatedServer,
	P2P
};

UENUM(BlueprintType)
enum EResolveServerAddress : int
{
	Default,
	Dynamic
};

USTRUCT(BlueprintType, Blueprintable)
struct MODULENETWORKEOS_API FPlayerStateCopyProperties
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 PlayerId;

	UPROPERTY(BlueprintReadWrite)
	int32 LobbyId;
};

USTRUCT(BlueprintType)
struct MODULENETWORKEOS_API FNetworkData
{
	GENERATED_BODY()

	FNetworkData(){};
	~FNetworkData(){};

	UPROPERTY()
	FName SessionName;

	ESessionState SessionState = ESessionState::Session_None;

	ENetRole NetRole = ROLE_None;

	EServerType ServerType = DedicatedServer;
};
#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Network/Types/RequestDTO.h"
#include "LobbyInfo.generated.h"


USTRUCT(BlueprintType)
struct MODULENETWORKEOS_API FOnlineInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FRequestDTO RequestDto;

	UPROPERTY(BlueprintReadWrite)
	FName SessionName;

	UPROPERTY(BlueprintReadWrite)
	bool bUseEOSRegister = false;

	UPROPERTY(BlueprintReadWrite)
	bool bValid = false;
};

USTRUCT(BlueprintType, Blueprintable)
struct MODULENETWORKEOS_API FLobbyInfo : public FOnlineInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bIsOwner = false;

	UPROPERTY(BlueprintReadWrite)
	FString LobbyId;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> membersName;

	void Clear();
};

inline void FLobbyInfo::Clear()
{
	bValid = false;
	bUseEOSRegister = true;
	bIsOwner = true;
	LobbyId = "";
	SessionName = "";
	membersName.Reserve(0);
}

USTRUCT(BlueprintType, Blueprintable)
struct MODULENETWORKEOS_API FSessionInfo : public FOnlineInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SessionId;

	UPROPERTY(BlueprintReadWrite)
	bool bRestartEnd = false;

	void clear()
	{
		SessionId = "";
		bValid = false;
		bUseEOSRegister = false;
		SessionName = "";
		bRestartEnd = false;
	}
};

USTRUCT()
struct MODULENETWORKEOS_API FEOSSessionSearchResult
{
	GENERATED_BODY()

	FOnlineSessionSearchResult SessionSearchResult;

	FSessionInfo SessionInfo;
};

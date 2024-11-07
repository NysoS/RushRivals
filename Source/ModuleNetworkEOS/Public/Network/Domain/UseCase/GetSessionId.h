#pragma once

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

struct GetSessionId
{
	static FString invoke(const IOnlineSessionPtr& SessionPtr, const FName& SessionName)
	{
		check(SessionPtr != nullptr);

		if (const FNamedOnlineSession* NamedOnlineSession = SessionPtr->GetNamedSession(SessionName))
		{
			return NamedOnlineSession->GetSessionIdStr();
		}

		return FString("");
	}
};

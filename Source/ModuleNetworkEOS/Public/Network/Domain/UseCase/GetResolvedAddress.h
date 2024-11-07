#pragma once

#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Network/Types/Request.h"

struct Resolver
{
	FString GetAddress(const IOnlineSessionPtr& SessionPtr, const FName& SessionName)
	{
		if (!SessionPtr)
		{
			return "";
		}

		SessionPtr->GetResolvedConnectString(SessionName, Address);

		return Address;
	}

	bool GetRequestByAddress(const IOnlineSessionPtr& SessionPtr, const FName& SessionName, Request& RequestData)
	{
		if (!SessionPtr)
		{
			return false;
		}

		if (!SessionPtr->GetResolvedConnectString(SessionName, Address))
		{
			return false;
		}
	
		RequestData = Request(Address, "");

		return false;
	}

private:
	FString Address;
};

struct DefaultResolverType
{};

struct DynamicResolverType
{};

template<typename ResolverAddressType>
struct GetResolvedAddress
{
};

template<>
struct GetResolvedAddress<DefaultResolverType>
{
	static Request invoke(const IOnlineSessionPtr& SessionPtr, const FName& SessionName)
	{
		Request request;
		Resolver resolver;

		resolver.GetRequestByAddress(SessionPtr, SessionName, request);

		return request;
	}
};

template<>
struct GetResolvedAddress<DynamicResolverType>
{
	static Request invoke(const IOnlineSessionPtr& SessionPtr, const FName& SessionName)
	{
		Request request;
		Resolver resolver;

		if (resolver.GetRequestByAddress(SessionPtr, SessionName, request))
		{
			if (const FOnlineSessionSettings* setting = SessionPtr->GetSessionSettings(SessionName))
			{
				FString port;
				setting->Get(FName(UEOSSessionSubsystem::SESSION_SETTINGS_PORT), port);
				request.setPort(port);
			}
		}

		return request;
	}
};

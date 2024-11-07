#include "Network/Types/Request.h"

#include "ModuleNetworkEOSModules.h"
#include "Misc/RuntimeErrors.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"

Request::Request()
	: m_ip(""), m_port(""), m_params({}), bIsValid(false)
{
}

Request::Request(const FString& address, const FString& params)
	: m_ip(""), m_port(""), m_params({}), bIsValid(false)
{
	address.Split(":", &m_ip, &m_port, ESearchCase::CaseSensitive);
	if (!params.IsEmpty())
		parseParams(params);
}

Request::~Request()
{
}

FString Request::getIp() const
{
	return m_ip;
}

void Request::setIp(const FString& ip)
{
	m_ip = ip;
}

FString Request::getAddress() const
{
	return m_ip + ":" + m_port;
}

FString Request::getPort() const
{
	return m_port;
}

void Request::setPort(const FString& port)
{
	m_port = port;
}

FString Request::getUrl() const
{
	FString Path = getAddress();
	for(const auto& [Key, Value] : m_params)
	{
		Path += FString("?"+Key+"="+Value);
	}

	return Path;
}

bool Request::isValid() const
{
	return !getAddress().IsEmpty();
}

void Request::addParams(const FString& name, const FString& value)
{
	if (m_params.Contains(name))
	{
		LogRuntimeError(FText::FromString("Duplicate name parameters"));
	}

	m_params.Add(name, value);
}

void Request::parseParams(const FString& params)
{
	TArray<FString> options;
	params.ParseIntoArray(options, TEXT("?"));

	for (const FString option : options)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("op : %s"), *option);
		FString name, value;
		if (option.Split("=", &name, &value))
		{
			addParams(name, value);
		}
	}
}

TMap<FString, FString> Request::getParams() const
{
	return m_params;
}

FString Request::getParamsToString() const
{
	FString params;
	for (const auto& [key, value] : m_params)
	{
		params += FString("?" + key + "=" + value);
	}

	return params;
}

FRequestDTO Request::makeDTO(const Request& request)
{
	return FRequestDTO{ request.getIp(), request.getPort(), request.getParamsToString(), request.getUrl() };
}

Request Request::makeRequestFromDTO(const FRequestDTO& requestDto)
{
	Request request;

	request.setIp(requestDto.ip);
	request.setPort(requestDto.port);
	request.parseParams(requestDto.params);

	return request;
}

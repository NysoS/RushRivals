#pragma once
#include "RequestDTO.h"

template <typename ValueType>
struct ParamConverter
{
	static ValueType invoke(const FString& value)
	{
		static_assert(false, "ValueType is not supported yet!");
		return ValueType{};
	}
};

template <>
struct ParamConverter<FString>
{
	static FString invoke(const FString& value)
	{
		return value;
	}
};

template <>
struct ParamConverter<int32>
{
	static int32 invoke(const FString& value)
	{
		if (value.IsEmpty())
		{
			return INDEX_NONE;
		}

		return FCString::Atoi(*value);
	}
};

class Request
{
public:
	Request();
	Request(const FString& address, const FString& params);
	~Request();

	FString getPort() const;
	void setPort(const FString& port);
	FString getIp() const;
	void setIp(const FString& ip);
	FString getAddress() const;
	FString getUrl() const;

	bool isValid() const;

	void addParams(const FString& name, const FString& value);
	void parseParams(const FString& params);

	template<typename ValueType>
	ValueType getValueParameter(const FString& parameter);
	TMap<FString, FString> getParams() const;
	FString getParamsToString() const;

	static FRequestDTO makeDTO(const Request& request);
	static Request makeRequestFromDTO(const FRequestDTO& requestDto);

protected:
	FString m_ip;
	FString m_port;
	TMap<FString, FString> m_params;
	bool bIsValid;
};

template <typename ValueType>
ValueType Request::getValueParameter(const FString& parameter)
{
	const auto& value = m_params.Find(parameter);

	if (!value)
	{
		return ValueType{};
	}

	return ParamConverter<ValueType>::invoke(*value);
}
#pragma once

#include "CacheObjectBase.h"

template <typename CacheDataType>
class CacheObject : public CacheObjectBase
{
public:
	CacheObject(const CacheDataType& data);
	virtual ~CacheObject() override = default;

	CacheDataType value;
};

template <typename CacheDataType>
CacheObject<CacheDataType>::CacheObject(const CacheDataType& data)
	: value(data)
{
}

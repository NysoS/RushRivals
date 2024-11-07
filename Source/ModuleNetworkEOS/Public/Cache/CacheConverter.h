#pragma once
#include "CacheObject.h"
#include "CacheObjectBase.h"

template <typename Type>
struct CacheConverter
{
	static CacheObjectBase* convert(const Type& input)
	{
		return new CacheObject<Type>(input);
	}

	static Type revert(CacheObjectBase* input)
	{
		CacheObject<Type>* cacheObject = static_cast<CacheObject<Type>*>(input);

		return cacheObject->value;
	}
};

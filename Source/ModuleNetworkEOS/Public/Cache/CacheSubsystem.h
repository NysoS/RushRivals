// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cassert>

#include "CoreMinimal.h"
#include "CacheConverter.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CacheSubsystem.generated.h"

class CacheObjectBase;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API UCacheSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	template <typename Type>
	void Set(const FString& key, const Type& value);

	template <typename Type>
	bool Get(const FString& key, Type& returnValue);

	void Clear(const FString& key);
	void CacheClear();

protected:
	TMap<FString, CacheObjectBase*> m_data;
};

template <typename Type>
void UCacheSubsystem::Set(const FString& key, const Type& value)
{
	if (m_data.Contains(key))
	{
		//Type oldValue = m_data.FindRef(key);
		//oldValue += value;
		//m_data.Emplace(key);
	}

	m_data.Emplace(key, CacheConverter<Type>::convert(value));
}

template <typename Type>
bool UCacheSubsystem::Get(const FString& key, Type& returnValue)
{
	if (!m_data.Contains(key))
	{
		return false;
	}
	
	returnValue = CacheConverter<Type>::revert(m_data.FindRef(key));
	return true;
}

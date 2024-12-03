// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameInstance/EOSGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "HBGameInstance.generated.h"

class UHBSaveGame;

/**
 * 
 */
UCLASS()
class HB_API UHBGameInstance : public UEOSGameInstance
{
	GENERATED_BODY()

public:

	template <typename type>
	void SaveGame(type SaveGamePtr, const FString& SaveName, const int32& SaveIndex)
	{
		check(SaveGamePtr);
		// TODO: rework
		//UGameplayStatics::SaveGameToSlot(SaveGamePtr, SaveName, SaveIndex);
	}

	template <typename type>
	type* GetSaveGame(const FString& SaveName, const int32& SaveIndex)
	{
		if (UGameplayStatics::DoesSaveGameExist(SaveName, SaveIndex))
		{
			return Cast<type>(UGameplayStatics::LoadGameFromSlot(SaveName, SaveIndex));
		}
		else
		{
			return Cast<type>(UGameplayStatics::CreateSaveGameObject(type::StaticClass()));
		}
	}

};

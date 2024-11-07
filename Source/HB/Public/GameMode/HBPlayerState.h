// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "Character/Utils/HBUtility.h"
#include "HBPlayerState.generated.h"

struct FDataSkin;
class UHBSaveGame;

/**
 * 
 */
UCLASS()
class HB_API AHBPlayerState : public AEOSPlayerState
{
	GENERATED_BODY()

public:

	AHBPlayerState();

	UFUNCTION()
	void OnSkinChange(const FString& key, const FName& data);

	UFUNCTION()
	void LoadSkinData();

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, SaveGame)
	TMap<FString, FName> SkinListData;

	UPROPERTY(Replicated)
	FString SkinListSerialized;

private:

	UPROPERTY()
	FString SkinSaveName;

	UPROPERTY()
	int32 SkinSaveIndex;

	UPROPERTY()
	TObjectPtr<UHBSaveGame> m_SkinSaveGame;
	
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HBPlayerState.h"

#include "Character/HBCharacter.h"
#include "GameMode/HBGameInstance.h"
#include "GameMode/HBSaveGame.h"
#include "Net/UnrealNetwork.h"
#include "Widget/Skin/Enum_SkinPart.h"

DEFINE_LOG_CATEGORY_STATIC(LogHBPlayerState, Log, All)

#define PRINTLOG(text, ...) UE_LOG(LogHBPlayerState, Display, TEXT(text), ##__VA_ARGS__)

AHBPlayerState::AHBPlayerState()
	: Super()
	, SkinSaveName("SkinSaveData")
	, SkinSaveIndex(0)
{
	SkinListLib::SetAllValue(SkinListData, {"Default"});
}

void AHBPlayerState::OnSkinChange(const FString& key, const FName& data)
{
	SkinListData[key] = data;
	SkinListSerialized = SkinListLib::SerializeMap(SkinListData);
	GetPawn<AHBCharacter>()->UpdateSkin(SkinListSerialized);

	if (TObjectPtr<UHBGameInstance> CurrentGameInstance = GetGameInstance<UHBGameInstance>())
	{
		m_SkinSaveGame->SelectedSkin[key] = data;
		CurrentGameInstance->SaveGame(m_SkinSaveGame, SkinSaveName, SkinSaveIndex);
	}
}

void AHBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, SkinListSerialized);
}

void AHBPlayerState::LoadSkinData()
{
	PRINTLOG("Skin name : %s, %d", *SkinSaveName, SkinSaveIndex)
	if (TObjectPtr<UHBGameInstance> CurrentGameInstance = GetGameInstance<UHBGameInstance>())
	{
		m_SkinSaveGame = CurrentGameInstance->GetSaveGame<UHBSaveGame>(SkinSaveName, SkinSaveIndex);
		SkinListData = m_SkinSaveGame->SelectedSkin;
	}
	else
	{
		SkinListLib::SetAllValue(SkinListData, {"Default"});
	}
	SkinListSerialized = SkinListLib::SerializeMap(SkinListData);
}

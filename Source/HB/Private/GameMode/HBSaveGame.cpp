// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/HBSaveGame.h"
#include "Character/Utils/HBUtility.h"

UHBSaveGame::UHBSaveGame() : Super()
{
	SkinListLib::SetAllValue(SelectedSkin, {"Default"});
}

// Copyright Epic Games, Inc. All Rights Reserved.

/**
* @file HBGameMode.cpp
* @brief
*
* @author HitBoxTM
*
* @copyright (c) HitBoxTM 2024
* @date 26/10/2023
* @version 1.0
*/
#include "Gamemode/HBGameMode.h"
#include "UObject/ConstructorHelpers.h"

/**
* @fn AHBGameMode
* @brief 
*/
AHBGameMode::AHBGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

/**
* @file HB.h
* @brief
*
* @author HitBoxTM
*
* @copyright (c) HitBoxTM 2024
* @date 26/10/2023
* @version 1.0
*/
#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(HBGame, Log, All);

#define WARNING_LOG(Format, ...) \
		UE_LOG(HBGame, Warning, Format, ##__VA_ARGS__)

#define ERROR_LOG(Format, ...) \
		UE_LOG(HBGame, Error, Format, ##__VA_ARGS__)

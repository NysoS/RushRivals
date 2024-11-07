// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/ModuleNetworkEOSSettings.h"

#if WITH_EDITOR
void UModuleNetworkEOSSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UModuleNetworkEOSSettings, GameRulesDataTable))
	{
		TArray<FName> ar = GetGameModeInstanceType();
		GameModeIntanceSelected = !ar.IsEmpty() ? ar[0] : FName("");
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
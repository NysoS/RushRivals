// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EOSTeamDisplayAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API UEOSTeamDisplayAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TeamId;

	UPROPERTY(EditAnywhere,	BlueprintReadOnly)
	FText TeamName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor ColorParameter;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

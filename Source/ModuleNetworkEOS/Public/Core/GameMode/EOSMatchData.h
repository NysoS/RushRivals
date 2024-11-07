// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EOSMatchData.generated.h"

UCLASS()
class MODULENETWORKEOS_API AEOSMatchData : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEOSMatchData();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Replicated)
	float TimerMatch;

	UPROPERTY(Replicated)
	bool bIsStarting;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameMode/EOSMatchData.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/GameMode/EOSGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values	
AEOSMatchData::AEOSMatchData()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AEOSMatchData::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEOSMatchData::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEOSMatchData, TimerMatch);
	DOREPLIFETIME(AEOSMatchData, bIsStarting);
}

// Called every frame
void AEOSMatchData::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bIsStarting)
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("Client Timer remaining %f"), TimerMatch);
	}
}


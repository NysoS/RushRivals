// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Character/EOSCharacter.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/Library/NetworkEOSBPFunctionLibrary.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Teams/EOSTeamInfo.h"
#include "Teams/EOSTeamSubsystem.h"

AEOSCharacter::AEOSCharacter(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEOSCharacter::AssignOutlineColor(const int32& IndexColor)
{
	
}

// Called when the game starts or when spawned
void AEOSCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AEOSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEOSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEOSCharacter::InputDisable(bool bDisable)
{
	AEOSPlayerController* EOSPlayerController = UNetworkEOSBPFunctionLibrary::GetPlayerController(GetWorld());
	if (!EOSPlayerController)
	{
		return;
	}

	if (bDisable)
	{
		DisableInput(EOSPlayerController);
	}
	else
	{
		EnableInput(EOSPlayerController);
	}
}


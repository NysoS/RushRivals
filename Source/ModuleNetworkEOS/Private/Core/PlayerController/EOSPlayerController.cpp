// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PlayerController/EOSPlayerController.h"

#include "Core/GameState/EOSGameState.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Teams/EOSTeamCreationComponent.h"

AEOSPlayerController::AEOSPlayerController()
{
	bReplicates = true;
	bActorSeamlessTraveled = true;
}

void AEOSPlayerController::OnRep_NetMulticastResetControlRotation_Implementation(const FRotator& NewRotation)
{
	if(GetRemoteRole() != ROLE_Authority)
		return;

	GetPawn()->SetActorRotation(NewRotation);
	SetControlRotation(NewRotation);
}

void AEOSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AEOSPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AEOSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, TeamId);
}

void AEOSPlayerController::BeginReplication()
{
	Super::BeginReplication();

	AEOSGameState* GameState = GetWorld()->GetGameState<AEOSGameState>();
	if(!GameState)
	{
		return;
	}

	UEOSTeamCreationComponent* TeamCreationComponent = GameState->GetComponentByClass<UEOSTeamCreationComponent>();
	if (TeamCreationComponent)
	{
		TeamCreationComponent->TeamPopulate();
	}
}
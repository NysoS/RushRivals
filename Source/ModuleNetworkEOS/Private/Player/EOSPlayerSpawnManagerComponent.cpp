// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/EOSPlayerSpawnManagerComponent.h"

#include "Core/Library/NetworkEOSBPFunctionLibrary.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "Core/PlayerStart/EOSPlayerStart.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UEOSPlayerSpawnManagerComponent::UEOSPlayerSpawnManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UEOSPlayerSpawnManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEOSPlayerStart::StaticClass(), m_PlayerStarts);
}

// Called every frame
void UEOSPlayerSpawnManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

TArray<AActor*> UEOSPlayerSpawnManagerComponent::GetPlayerStarts() const
{
	return m_PlayerStarts;
}

AEOSPlayerStart* UEOSPlayerSpawnManagerComponent::GetPlayerStartById(int32 PlayerStartId)
{
	for (AActor* PlayerStartActor : m_PlayerStarts)
	{
		AEOSPlayerStart* PlayerStart = Cast<AEOSPlayerStart>(PlayerStartActor);
		if (!PlayerStart)
		{
			continue;
		}

		if (PlayerStart->TeamId != PlayerStartId)
		{
			continue;
		}

		if (PlayerStart->bPlayerSpawned)
		{
			continue;
		}

		return PlayerStart;
	}

	return nullptr;
}

void UEOSPlayerSpawnManagerComponent::MovePlayerToSpawn(const AEOSPlayerState* PlayerState)
{
	if(!PlayerState)
		return;

	AEOSPlayerController* EOSPlayerController = Cast<AEOSPlayerController>(PlayerState->GetPlayerController());
	if (!EOSPlayerController)
		return;

	APawn* Pawn = EOSPlayerController->GetPawn();
	if(!Pawn)
		return;

	AEOSPlayerStart* PlayerStart = GetPlayerStartById(PlayerState->TeamIdN);
	if(!PlayerStart)
		return;

	PlayerStart->bPlayerSpawned = true;
	Pawn->SetActorLocationAndRotation(PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation());
	EOSPlayerController->OnRep_NetMulticastResetControlRotation(PlayerStart->GetActorRotation());

#if !UE_SERVER
	EOSPlayerController->SetControlRotation(PlayerStart->GetActorRotation());
#endif

}

void UEOSPlayerSpawnManagerComponent::ResetPlayerStart()
{
	for (AActor* PlayerStartActor : m_PlayerStarts)
	{
		AEOSPlayerStart* PlayerStart = Cast<AEOSPlayerStart>(PlayerStartActor);
		if (!PlayerStart)
		{
			continue;
		}

		PlayerStart->bPlayerSpawned = false;
	}
}

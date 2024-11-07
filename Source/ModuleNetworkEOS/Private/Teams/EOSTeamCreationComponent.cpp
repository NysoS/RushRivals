// Fill out your copyright notice in the Description page of Project Settings.


#include "Teams/EOSTeamCreationComponent.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/Character/EOSCharacter.h"
#include "Core/GameMode/EOSGameMode.h"
#include "Core/GameMode/EOSGameRule.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/Library/NetworkEOSBPFunctionLibrary.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "Core/PlayerStart/EOSPlayerStart.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Teams/EOSTeamDisplayAsset.h"
#include "Teams/EOSTeamInfo.h"
#include "Teams/EOSTeamSubsystem.h"


UEOSTeamCreationComponent::UEOSTeamCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	m_TeamInfoClass = AEOSTeamInfo::StaticClass();
}

// Called when the game starts
void UEOSTeamCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		ServerCreationTeam();

#if !UE_SERVER
		UWorld* World = GetWorld();
		if(World)
		{
			if(AEOSPlayerController* PlayerController = Cast<AEOSPlayerController>(World->GetFirstPlayerController()))
			{
				AffectationTeam(PlayerController);
			}
		}
#endif

	}
}

void UEOSTeamCreationComponent::ServerCreationTeam()
{
	for(const auto& KVP : m_TeamsToCreate)
	{
		const int32 TeamId = KVP.Key;
		ServerCreateTeam(TeamId, KVP.Value);
	}
}

void UEOSTeamCreationComponent::AffectationTeam(AEOSPlayerController* NewPlayer)
{
	UWorld* World = GetWorld();
	check(World);

	int32 TeamId = GetLastTeamAffected();

	if(TeamId != INDEX_NONE)
	{
		const UEOSTeamSubsystem* TemSubsystem = World->GetSubsystem<UEOSTeamSubsystem>();
		if(!TemSubsystem)
		{
			return;
		}

		AEOSTeamInfo* TeamInfo = TemSubsystem->GetTeamInfoWithTeamId(TeamId);
		if(!TeamInfo)
		{
			return;
		}

		AEOSPlayerState* PlayerState = NewPlayer->GetPlayerState<AEOSPlayerState>();
		if(!PlayerState)
		{
			return;
		}
		
		NewPlayer->TeamId = TeamId;
		PlayerState->TeamIdN = TeamId;
		PlayerState->TeamDisplayAsset = TeamInfo->GetTeamDisplayAsset();
	}

}

void UEOSTeamCreationComponent::RemoveAffectationTeam(AEOSPlayerController* Player)
{
	UWorld* World = GetWorld();
	check(World);

	const UEOSTeamSubsystem* TemSubsystem = World->GetSubsystem<UEOSTeamSubsystem>();
	if (!TemSubsystem)
	{
		return;
	}

	AEOSTeamInfo* TeamInfo = TemSubsystem->GetTeamInfoWithTeamId(Player->TeamId);
	if (!TeamInfo)
	{
		return;
	}
}

void UEOSTeamCreationComponent::SetOwnerShip(AActor* Actor)
{
	GetOwner()->SetOwner(Actor);
}

void UEOSTeamCreationComponent::ServerCreateTeam(int32 TeamId, UEOSTeamDisplayAsset* DisplayAsset)
{
	UWorld* World = GetWorld();
	check(World);

	if(m_TeamIdAlreadySpawnRegistery.Contains(TeamId))
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("TeamId already register"));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEOSTeamInfo* TeamInfo = World->SpawnActor<AEOSTeamInfo>(m_TeamInfoClass, SpawnParameters);
	if(!TeamInfo)
	{
		ERROR_LOG(TEXT("Failed to create team info actor from class"))
	}
	TeamInfo->SetTeamId(TeamId);
	TeamInfo->SetDisplayAsset(DisplayAsset);

	if (const AEOSGameMode* GameMode = GetWorld()->GetAuthGameMode<AEOSGameMode>())
	{
		const FEOSGameRule GameRule = GameMode->GetGameRules();
		TeamInfo->SetTeamMaxScore(GameRule.MaxRound);
	}
	
	TeamInfo->TryRegisterWithTeamSubsytem();

	m_TeamIdAlreadySpawnRegistery.Add(TeamId);
}

int32 UEOSTeamCreationComponent::GetLastTeamAffected()
{
	if (m_TeamsToCreate.Num() <= 0)
	{
		return INDEX_NONE;
	}

	if (m_LastTeamIdAffectation == INDEX_NONE)
	{
		m_LastTeamIdAffectation = m_TeamsToCreate.begin().Key();
	}

	if (m_LastTeamIdAffectation < m_TeamsToCreate.Num())
	{
		m_LastTeamIdAffectation++;
	}else
	{
		m_LastTeamIdAffectation = m_TeamsToCreate.begin().Key();
	}

	return m_LastTeamIdAffectation;
}


// Called every frame
void UEOSTeamCreationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UEOSTeamCreationComponent::TeamPopulate()
{
	if(GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		OnTeamPopulate();
	}else
	{
		FTimerHandle TeamPopulateTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TeamPopulateTimerHandle, this, &UEOSTeamCreationComponent::OnTeamPopulate, 4.f, false);
	}
}

void UEOSTeamCreationComponent::OnTeamPopulate()
{
	AEOSGameState* GameState = GetWorld()->GetGameState<AEOSGameState>();
	if (!GameState)
	{
		return;
	}

	TArray<AActor*> TeamInfoSpawned;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEOSTeamInfo::StaticClass(), TeamInfoSpawned);

	for (const auto& actor : TeamInfoSpawned)
	{
		AEOSTeamInfo* teamInfo = Cast<AEOSTeamInfo>(actor);
		if (!teamInfo)
		{
			continue;
		}

		teamInfo->RemoveMembersAll();
		for (APlayerState* playerState : GameState->PlayerArray)
		{
			AEOSPlayerState* EOSPlayerState = Cast<AEOSPlayerState>(playerState);
			if (!EOSPlayerState)
			{
				continue;
			}

			if (teamInfo->GetTeamId() == EOSPlayerState->TeamIdN)
			{
				teamInfo->AddMember(EOSPlayerState);
			}
		}

		for (const auto& info : teamInfo->GetMembers())
		{
			if (info)
			{
				WARNING_LOG(TEXT("PLAYER ON THE TEAM : %s"), *info->GetPlayerName());
			}
		}
	}

	const AEOSPlayerState* CurrentPlayerState = UNetworkEOSBPFunctionLibrary::GetCurrentPlayerState();
	if (!CurrentPlayerState)
		return;


	for (APlayerState* playerState : GameState->PlayerArray)
	{
		AEOSPlayerState* EOSPlayerState = Cast<AEOSPlayerState>(playerState);
		if (!EOSPlayerState)
		{
			continue;
		}

		int32 IndexColor = CurrentPlayerState->TeamIdN == EOSPlayerState->TeamIdN ? 1 : 2;

		AssignTeamColor(EOSPlayerState, IndexColor);
	}
}

void UEOSTeamCreationComponent::AssignTeamColor(AEOSPlayerState* PlayerState, const int32& IndexColor)
{
	APawn* Pawn = PlayerState->GetPawn();
	if (!Pawn)
		return;


	const AEOSCharacter* Character = Cast<AEOSCharacter>(Pawn);
	if (!Character)
		return;

	TArray<USkeletalMeshComponent*> Meshs;
	Character->GetComponents(Meshs, true);
	for (auto ActorComponent : Meshs)
	{
		ActorComponent->SetCustomDepthStencilValue(IndexColor);
		ActorComponent->SetRenderCustomDepth(true);
	}
}

void UEOSTeamCreationComponent::OnRep_NetMulticastTeamPopulate_Implementation()
{
	TeamPopulate();
}

void UEOSTeamCreationComponent::OnRep_ServerOnNewPlayerLogin_Implementation()
{
	TeamPopulate();
}


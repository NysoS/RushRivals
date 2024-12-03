// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameState/EOSTeamComponent.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/GameInstance/EOSGameInstance.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/PlayerController/EOSPlayerController.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Server/ServerInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

class UEOSGameInstance;
// Sets default values for this component's properties
UEOSTeamComponent::UEOSTeamComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	if (const UEOSGameInstance* OwnerGameInstance = Cast<UEOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		FName KeyRule = OwnerGameInstance->m_ServerInstance->GetServerInstanceName();
		FEOSGameRule* GameRule = OwnerGameInstance->m_ServerInstance->GetRule(KeyRule);

		if (GameRule)
		{
			m_CurrentGameRule = *GameRule;
		}
	}
}


// Called when the game starts
void UEOSTeamComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), playerStart);
}

void UEOSTeamComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEOSTeamComponent, TeamA);
	DOREPLIFETIME(UEOSTeamComponent, TeamB);
}


// Called every frame
void UEOSTeamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UEOSTeamComponent::CreateTeam()
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Team Creation Process"));

	TeamA.name = "TeamA";
	TeamA.teamId = ETeamId::TeamA;

	TeamB.name = "TeamB";
	TeamB.teamId = ETeamId::TeamB;
}

void UEOSTeamComponent::TeamAffectation()
{
	AEOSGameState* GameState = Cast<AEOSGameState>(GetOwner());
	if(!GameState)
	{
		return;
	}

	const size_t PlayerCount = GameState->PlayerArray.Num();

	bool bFirstTeam = true;
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("AFFECTE"));
	for(size_t i = 0; i < PlayerCount; ++i)
	{
		if(TeamA.members.Num() < m_CurrentGameRule.MaxTeamMember && bFirstTeam)
		{
			AEOSPlayerState* EOSPlayerState = static_cast<AEOSPlayerState*>(GameState->PlayerArray[i]);

			TeamA.members.Add(EOSPlayerState);
			bFirstTeam = false;
			continue;
		}

		if (TeamB.members.Num() < m_CurrentGameRule.MaxTeamMember && !bFirstTeam)
		{
			AEOSPlayerState* EOSPlayerState = static_cast<AEOSPlayerState*>(GameState->PlayerArray[i]);

			TeamB.members.Add(EOSPlayerState);
			bFirstTeam = true;
		}
	}
}

void UEOSTeamComponent::TeamAddNewPlayer(AEOSPlayerState* NewPlayerState)
{
	if(TeamA.members.Contains(NewPlayerState) || TeamB.members.Contains(NewPlayerState))
	{
		UE_LOG(ModuleNetworkEOS, Warning, TEXT("ALREADY CONTAINS: %s"), *NewPlayerState->GetPlayerName());

		return;
	}

	UE_LOG(ModuleNetworkEOS, Warning, TEXT("AFFECTE NEW PLAYER : %s"), *NewPlayerState->GetPlayerName());

	if (TeamA.members.Num() < m_CurrentGameRule.MaxTeamMember && !m_bAffectTeamFlipFlop)
	{
		TeamA.members.Add(NewPlayerState);
		CurrentTeamId = TeamA.teamId;
		m_bAffectTeamFlipFlop = true;

		return;
	}

	if (TeamB.members.Num() < m_CurrentGameRule.MaxTeamMember && m_bAffectTeamFlipFlop)
	{
		TeamB.members.Add(NewPlayerState);
		CurrentTeamId = TeamB.teamId;
		m_bAffectTeamFlipFlop = false;
	}
}

const FTeamData& UEOSTeamComponent::GetTeamById(const ETeamId& TeamId) const
{
	if(TeamId == ETeamId::TeamA)
	{
		return TeamA;
	}

	if (TeamId == ETeamId::TeamB)
	{
		return TeamB;
	}

	return *static_cast<FTeamData*>(nullptr);
}

const TArray<FTeamData> UEOSTeamComponent::GetTeams() const
{
	TArray<FTeamData> Teams;
	Teams.Add(TeamA);
	Teams.Add(TeamB);
	return Teams;
}

const FTeamData& UEOSTeamComponent::GetCurrentTeam() const
{
	return GetTeamById(CurrentTeamId);
}

void UEOSTeamComponent::SetResultRoundOrMatch()
{
	if(TeamA.score > TeamB.score)
	{
		TeamA.teamResultEndMatch = EMatchResult::Victory;
		TeamB.teamResultEndMatch = EMatchResult::Defeat;
		return;
	}

	if (TeamB.score > TeamA.score)
	{
		TeamA.teamResultEndMatch = EMatchResult::Defeat;
		TeamB.teamResultEndMatch = EMatchResult::Victory;
	}
}

void UEOSTeamComponent::Test()
{
	if (AEOSGameState* GameState = Cast<AEOSGameState>(GetOwner()))
	{
		AEOSPlayerController* EOSPlayerController = Cast<AEOSPlayerController>(GetWorld()->GetFirstPlayerController());
		if (!EOSPlayerController)
		{
			return;
		}

		AEOSPlayerState* playerState = EOSPlayerController->GetPlayerState<AEOSPlayerState>();
		if (playerState)
		{
			if(TeamA.members.Contains(playerState))
			{
				CurrentTeamId = TeamA.teamId;
			}
			else if(TeamB.members.Contains(playerState))
			{
				CurrentTeamId = TeamB.teamId;
			}
		}

		GameState->OnTeamsUpdate.Broadcast();
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Teams/EOSTeamInfo.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Teams/EOSTeamSubsystem.h"

AEOSTeamInfo::AEOSTeamInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), m_TeamDisplayAsset(nullptr), m_TeamId(INDEX_NONE), m_MatchResult(EMatchResult::Draw), m_Score(0)
{
	bReplicates = true;
	bNetLoadOnClient = true;
	bAlwaysRelevant = true;
	NetPriority = 3.f;
}

void AEOSTeamInfo::BeginPlay()
{
	Super::BeginPlay();

	TryRegisterWithTeamSubsytem();
}

void AEOSTeamInfo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AEOSTeamInfo::BeginReplication()
{
	Super::BeginReplication();
}

void AEOSTeamInfo::SetDisplayAsset(TObjectPtr<UEOSTeamDisplayAsset> TeamDisplayAsset)
{
	m_TeamDisplayAsset = TeamDisplayAsset;
}

void AEOSTeamInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AEOSTeamInfo, m_TeamId, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AEOSTeamInfo, m_TeamDisplayAsset, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AEOSTeamInfo, m_MaxScore, COND_InitialOnly);
}

void AEOSTeamInfo::RegisterWithTeamSubsystem(UEOSTeamSubsystem* Subsystem)
{
	Subsystem->RegisterTeam(m_TeamId, this);
}

void AEOSTeamInfo::TryRegisterWithTeamSubsytem()
{
	if(m_TeamId != INDEX_NONE)
	{
		UEOSTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UEOSTeamSubsystem>();
		if(ensure(TeamSubsystem))
		{
			RegisterWithTeamSubsystem(TeamSubsystem);
		}
	}
}

void AEOSTeamInfo::AddMember(AEOSPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		UE_LOG(ModuleNetworkEOS, Error, TEXT("Can't add member to this team, because playerState is nullptr"))
		return;
	}

	if(m_TeamMembers.Contains(PlayerState))
	{
		return;
	}

	m_TeamMembers.Add(PlayerState);
	OnTeamMemberAdded.Broadcast(PlayerState->GetPlayerId());
}

void AEOSTeamInfo::RemoveMember(AEOSPlayerState* PlayerState)
{
	if(m_TeamMembers.Contains(PlayerState))
	{
		m_TeamMembers.Remove(PlayerState);
	}
}

void AEOSTeamInfo::RemoveMembersAll()
{
	m_TeamMembers.Reset();
}

void AEOSTeamInfo::SetTeamId(const int32& TeamId)
{
	m_TeamId = TeamId;
}

void AEOSTeamInfo::SetTeamScore(const int32& Score)
{
	m_Score = Score;
}

void AEOSTeamInfo::AddTeamScore(const int32& Score)
{
	m_Score += Score;
}

void AEOSTeamInfo::ResetTeamScore()
{
	m_Score = 0;
}

void AEOSTeamInfo::SetTeamMaxScore(const int32& MaxScore)
{
	m_MaxScore = MaxScore;
}

TEnumAsByte<EMatchResult> AEOSTeamInfo::GetResultMatch() const
{
	return m_MatchResult;
}

void AEOSTeamInfo::SetResultMatch(const int32& VictoryTeamId)
{
	if (VictoryTeamId == INDEX_NONE)
	{
		m_MatchResult = EMatchResult::Draw;
		OnResultMatchUpdated.Broadcast(m_TeamId, m_MatchResult, INDEX_NONE);
		return;
	}

	if (VictoryTeamId == m_TeamId)
	{
		m_MatchResult = EMatchResult::Victory;
		if (m_MaxScore > 0)
		{
			AddTeamScore(1);
		}
	}else
	{
		m_MatchResult = EMatchResult::Defeat;
	}

	OnTeamScoreUpdated.Broadcast(VictoryTeamId, m_Score);
	OnResultMatchUpdated.Broadcast(m_TeamId, m_MatchResult, VictoryTeamId);
}

void AEOSTeamInfo::ResetMatchResult()
{
	m_MatchResult = EMatchResult::Draw;
}

void AEOSTeamInfo::OnRep_TeamId()
{
	TryRegisterWithTeamSubsytem();
}

void AEOSTeamInfo::OnRep_TeamDisplayAsset()
{
	TryRegisterWithTeamSubsytem();
}

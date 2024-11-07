// Fill out your copyright notice in the Description page of Project Settings.


#include "Teams/EOSTeamSubsystem.h"

#include "Teams/EOSTeamInfo.h"

UEOSTeamSubsystem::UEOSTeamSubsystem()
{
}

void UEOSTeamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEOSTeamSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UEOSTeamSubsystem::RegisterTeam(int32 TeamId, AEOSTeamInfo* TeamInfo)
{
	if(m_TeamsRegister.Contains(TeamId))
	{
		return;
	}

	m_TeamsRegister.Add(TeamId, TeamInfo);
}

void UEOSTeamSubsystem::UnRegisterTeam(int32 TeamId)
{
	if (!m_TeamsRegister.Contains(TeamId))
	{
		return;
	}

	m_TeamsRegister.Remove(TeamId);
}

AEOSTeamInfo* UEOSTeamSubsystem::GetTeamInfoWithTeamId(int32 TeamId) const
{
	if(m_TeamsRegister.Num() <= 0)
	{
		return nullptr;
	}

	return *m_TeamsRegister.Find(TeamId);
}

const AEOSTeamInfo* UEOSTeamSubsystem::GetConstTeamInfoWithTeamId(int32 TeamId) const
{
	return GetTeamInfoWithTeamId(TeamId);
}

TMap<int32, AEOSTeamInfo*> UEOSTeamSubsystem::GetTeams() const
{
	return m_TeamsRegister;
}

AEOSTeamInfo* UEOSTeamSubsystem::GetTeamIdWithVictoryResult() const
{
	for (auto& [key, TeamInfo] : m_TeamsRegister)
	{
		if (!TeamInfo) continue;

		if (TeamInfo->GetResultMatch() == EMatchResult::Victory) return TeamInfo;
	}

	return nullptr;
}

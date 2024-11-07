// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/MatchState.h"
#include "Kismet/GameplayStatics.h"
#include "Online/MatchContext.h"
#include "Core/GameState/EOSGameState.h"

UMatchState::UMatchState()
{
	if(AEOSGameState* GameState = Cast<AEOSGameState>(UGameplayStatics::GetGameState(UObject::GetWorld())))
	{
		m_GameState = GameState;
	}
}

void UMatchState::SetContext(UMatchContext* context)
{
	if(m_MatchContext != nullptr)
	{
		m_MatchContext = nullptr;
	}

	m_MatchContext = context;
}

void UMatchState::InitState()
{
	m_MatchContext->matchMetaData.CycleStepState = EMatchStepState::Init;
}

void UMatchState::HandleState()
{
	m_MatchContext->matchMetaData.CycleStepState = EMatchStepState::Handling;
}

void UMatchState::ExitState()
{
	m_MatchContext->matchMetaData.CycleStepState = EMatchStepState::Exit;
}

void UMatchState::SwitchState(const TEnumAsByte<EMatchState>& state) const
{
	if (m_MatchContext)
	{
		m_MatchContext->UpdateState(state);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/MatchContext.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/GameState/EOSGameState.h"
#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"
#include "Online/MatchState.h"

UMatchContext::UMatchContext()
{
	if(AEOSGameState* gameState = Cast<AEOSGameState>(UGameplayStatics::GetGameState(UObject::GetWorld())))
	{
		m_GameState = gameState;
	}
}

void UMatchContext::SetDefaultState(UMatchState* defaultState)
{
	if(m_State != nullptr)
	{
		m_State = nullptr;
	}

	m_State = defaultState;
	m_State->SetContext(this);
	m_State->InitState();
}

void UMatchContext::TransitionTo(UMatchState* newState)
{
	if(newState == nullptr)
	{
		return;
	}

	if(newState == m_State)
	{
		return;
	}

	if(m_State != nullptr)
	{
		m_State->ExitState();
		m_State = nullptr;
	}

	m_State = newState;
	m_State->SetContext(this);
	m_State->InitState();
}

void UMatchContext::InitState() const
{
	if (m_State == nullptr)
	{
		return;
	}

	m_State->InitState();
}

void UMatchContext::ExecuteState() const
{
	if(m_State == nullptr)
	{
		return;
	}

	if(!m_State->bUseHandleTick)
	{
		return;
	}

	m_State->HandleState();
}

void UMatchContext::ExitState() const
{
	
}

void UMatchContext::UpdateState(const TEnumAsByte<EMatchState>& newState) const
{
	if (AEOSGameState* gameState = GetGameState())
	{
		gameState->UpdateCurrentMatchState(newState);
	}
}

UMatchState* UMatchContext::GetCurrentState() const
{
	if(!m_State)
	{
		return nullptr;
	}

	return m_State;
}

AEOSGameState* UMatchContext::GetGameState() const
{
	if(!m_GameState)
	{
		return nullptr;
	}

	return m_GameState;
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EOSTeamSubsystem.generated.h"

class AEOSTeamInfo;

/**
 * 
 */
UCLASS()
class MODULENETWORKEOS_API UEOSTeamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamMemberAdded, const int32&, TeamId, const FString&, PLayerName);

public:
	UEOSTeamSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void RegisterTeam(int32 TeamId, AEOSTeamInfo* TeamInfo);
	UFUNCTION(BlueprintCallable)
	void UnRegisterTeam(int32 TeamId);
	UFUNCTION(BlueprintCallable)
	AEOSTeamInfo* GetTeamInfoWithTeamId(int32 TeamId) const;

	UFUNCTION(BlueprintCallable)
	const AEOSTeamInfo* GetConstTeamInfoWithTeamId(int32 TeamId) const;

	UFUNCTION(BlueprintCallable)
	TMap<int32, AEOSTeamInfo*> GetTeams() const;

	UFUNCTION(BlueprintCallable)
	AEOSTeamInfo* GetTeamIdWithVictoryResult() const;

protected:
	UPROPERTY()
	TMap<int32, AEOSTeamInfo*> m_TeamsRegister;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Online/MatchState.h"
#include "EOSTeamInfo.generated.h"

class AEOSPlayerState;
class UEOSTeamSubsystem;
class UEOSTeamDisplayAsset;
/**
 * 
 */
UCLASS(BlueprintType)
class MODULENETWORKEOS_API AEOSTeamInfo : public AInfo
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamScoreUpdated, const int32&, TeamId, const int32&, Score);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FonTeamResultMatchUpdated, const int32&, TeamId, const EMatchResult&, MatchResult, const int32&, VictoryTeamId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamMemberAdded, const int32&, PlayerId);

public:
	UPROPERTY(BlueprintAssignable, Category = "EOS|Delegate|TeamInfoNotify")
	FOnTeamScoreUpdated OnTeamScoreUpdated;
	UPROPERTY(BlueprintAssignable, Category = "EOS|Delegate|TeamInfoNotify")
	FonTeamResultMatchUpdated OnResultMatchUpdated;
	UPROPERTY(BlueprintAssignable, Category = "EOS|Delegate|TeamInfoNotify")
	FOnTeamMemberAdded OnTeamMemberAdded;

	AEOSTeamInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	int32 GetTeamId() const { return m_TeamId; }
	UFUNCTION(BlueprintCallable)
	void SetTeamId(const int32& TeamId);
	UFUNCTION(BlueprintCallable)
	int32 GetTeamScore() const { return m_Score; }

	UFUNCTION(BlueprintCallable)
	TEnumAsByte<EMatchResult> GetResultMatch() const;

	UFUNCTION()
	void SetResultMatch(const int32& VictoryTeamId);
	UFUNCTION()
	void ResetMatchResult();

	UFUNCTION(BlueprintCallable)
	void SetTeamScore(const int32& Score);
	UFUNCTION(BlueprintCallable)
	void AddTeamScore(const int32& Score);
	UFUNCTION(BlueprintCallable)
	void ResetTeamScore();

	UFUNCTION()
	void SetTeamMaxScore(const int32& MaxScore);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void BeginReplication() override;

	UFUNCTION(BlueprintCallable)
	UEOSTeamDisplayAsset* GetTeamDisplayAsset() const { return m_TeamDisplayAsset; }
	void SetDisplayAsset(TObjectPtr<UEOSTeamDisplayAsset> TeamDisplayAsset);

	UFUNCTION()
	void TryRegisterWithTeamSubsytem();

	UFUNCTION()
	void AddMember(AEOSPlayerState* PlayerState);
	UFUNCTION()
	void RemoveMember(AEOSPlayerState* PlayerState);
	UFUNCTION()
	void RemoveMembersAll();

	TArray<TObjectPtr<AEOSPlayerState>> GetMembers() const { return m_TeamMembers; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	virtual void RegisterWithTeamSubsystem(UEOSTeamSubsystem* Subsystem);

private:
	UFUNCTION()
	void OnRep_TeamId();

	UFUNCTION()
	void OnRep_TeamDisplayAsset();

private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamId)
	int32 m_TeamId;

	UPROPERTY(ReplicatedUsing = OnRep_TeamDisplayAsset)
	TObjectPtr<UEOSTeamDisplayAsset> m_TeamDisplayAsset;

	UPROPERTY()
	TArray<TObjectPtr<AEOSPlayerState>> m_TeamMembers;

	UPROPERTY()
	TEnumAsByte<EMatchResult> m_MatchResult;

	UPROPERTY(Replicated)
	int32 m_MaxScore;

	UPROPERTY()
	int32 m_Score;
};

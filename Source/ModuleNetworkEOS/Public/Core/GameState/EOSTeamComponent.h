// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EOSGameState.h"
#include "ModuleNetworkEOSModules.h"
#include "Components/ActorComponent.h"
#include "Core/GameMode/EOSGameRule.h"
#include "Online/TeamData.h"
#include "EOSTeamComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MODULENETWORKEOS_API UEOSTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEOSTeamComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	virtual void CreateTeam();
	UFUNCTION()
	virtual void TeamAffectation();
	UFUNCTION()
	virtual void TeamAddNewPlayer(AEOSPlayerState* NewPlayerState);
	UFUNCTION()
	const FTeamData& GetTeamById(const ETeamId& TeamId) const;
	UFUNCTION(BlueprintCallable)
	const TArray<FTeamData> GetTeams() const;

	UFUNCTION(BlueprintCallable)
	const FTeamData& GetCurrentTeam() const;

	UFUNCTION()
	void SetResultRoundOrMatch();

	UFUNCTION()
	void Test();

	UPROPERTY(ReplicatedUsing=Test, BlueprintReadOnly, EditAnywhere)
	FTeamData TeamA;
	UPROPERTY(ReplicatedUsing = Test, BlueprintReadOnly, EditAnywhere)
	FTeamData TeamB;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<ETeamId> CurrentTeamId;

	UPROPERTY()
	TArray<AActor*> playerStart;

protected:
	UPROPERTY()
	FEOSGameRule m_CurrentGameRule;

	UPROPERTY()
	bool m_bAffectTeamFlipFlop;
};

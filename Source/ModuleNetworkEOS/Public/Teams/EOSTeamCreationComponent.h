// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EOSTeamCreationComponent.generated.h"

class AEOSPlayerController;
class AEOSPlayerState;
class UEOSTeamDisplayAsset;
class AEOSTeamInfo;

UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API UEOSTeamCreationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEOSTeamCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION()
	virtual void AffectationTeam(AEOSPlayerController* NewPlayer);
	UFUNCTION()
	virtual void RemoveAffectationTeam(AEOSPlayerController* Player);

	UFUNCTION()
	void SetOwnerShip(AActor* Actor);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

//#if WITH_SERVER_CODE
	virtual void ServerCreationTeam();

private:
	UFUNCTION()
	void ServerCreateTeam(int32 TeamId, UEOSTeamDisplayAsset* DisplayAsset);
	UFUNCTION()
	int32 GetLastTeamAffected();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
	void OnRep_ServerOnNewPlayerLogin();

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_NetMulticastTeamPopulate();

	UFUNCTION()
	void TeamPopulate();
	UFUNCTION()
	void OnTeamPopulate();

	UFUNCTION()
	void AssignTeamColor(AEOSPlayerState* PlayerState, const int32& IndexColor);

protected:
	UPROPERTY(EditDefaultsOnly, Category = Teams)
	TSubclassOf<AEOSTeamInfo> m_TeamInfoClass;

	UPROPERTY(EditDefaultsOnly, Category = "Teams")
	TMap<uint8, TObjectPtr<UEOSTeamDisplayAsset>> m_TeamsToCreate;

	UPROPERTY()
	TArray<uint8> m_TeamIdAlreadySpawnRegistery;

private:
	UPROPERTY()
	int32 m_LastTeamIdAffectation = INDEX_NONE;
};

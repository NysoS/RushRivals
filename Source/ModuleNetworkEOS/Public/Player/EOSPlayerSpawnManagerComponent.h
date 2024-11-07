// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EOSPlayerSpawnManagerComponent.generated.h"


class AEOSPlayerState;
class AEOSPlayerStart;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MODULENETWORKEOS_API UEOSPlayerSpawnManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEOSPlayerSpawnManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadWRite, EditAnywhere, Category="Spawn")
	TSubclassOf<AActor> PlayerStartSubclass;

	UFUNCTION()
	TArray<AActor*> GetPlayerStarts() const;

	UFUNCTION()
	virtual AEOSPlayerStart* GetPlayerStartById(int32 PlayerStartId);
	UFUNCTION()
	virtual void MovePlayerToSpawn(const AEOSPlayerState* PlayerState);
	UFUNCTION()
	void ResetPlayerStart();

private:
	UPROPERTY()
	TArray<AActor*> m_PlayerStarts;
};

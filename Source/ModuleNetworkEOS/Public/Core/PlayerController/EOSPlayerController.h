// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EOSPlayerController.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MODULENETWORKEOS_API AEOSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEOSPlayerController();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team Properties")
	int32 TeamId = INDEX_NONE;

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_NetMulticastResetControlRotation(const FRotator& NewRotation);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginReplication() override;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RPCSessionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MODULENETWORKEOS_API URPCSessionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URPCSessionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Client, Reliable)
	void Client_RPC_DestroySession(FName SessionName);

	UFUNCTION(Client, Reliable)
	void Client_RPC_LeaveSession(FName SessionName);

	UFUNCTION(Server, Reliable)
	void Server_RPC_LeaveSession();

protected:
	UPROPERTY()
	class AEOSGameMode* m_GameModePtr;
};

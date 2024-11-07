// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/NetworkSystem/RPC/RPCSessionComponent.h"

#include "ModuleNetworkEOSModules.h"
#include "Core/Library/NetworkEOSBPFunctionLibrary.h"
#include "Core/Subsystem/EOSSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Core/GameMode/EOSGameMode.h"

// Sets default values for this component's properties
URPCSessionComponent::URPCSessionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	if(AEOSGameMode* CurrentGameMode = Cast<AEOSGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		m_GameModePtr = CurrentGameMode;
	}
}


// Called when the game starts
void URPCSessionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void URPCSessionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void URPCSessionComponent::Client_RPC_DestroySession_Implementation(FName SessionName)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Destroy Session cause host disconnected"));
	UEOSSessionSubsystem* SessionSubsystem = UGameplayStatics::GetGameInstance(UNetworkEOSBPFunctionLibrary::GetActiveWorld())->GetSubsystem<UEOSSessionSubsystem>();
	if(!SessionSubsystem)
	{
		return;
	}
	
	SessionSubsystem->DestroySession(SessionName);
}

void URPCSessionComponent::Server_RPC_LeaveSession_Implementation()
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Server RPC leaveSession"));
}

void URPCSessionComponent::Client_RPC_LeaveSession_Implementation(FName SessionName)
{
	UE_LOG(ModuleNetworkEOS, Warning, TEXT("Client RPC leaveSession"));
}
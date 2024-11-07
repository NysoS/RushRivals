// Copyright Epic Games, Inc. All Rights Reserved.

/**
* @file AI_Fotch.h
* @brief Classe de base pour l'IA du Vif. 
*
* @author HitBoxTM
*
* @copyright (c) HitBoxTM 2024
* @date 07/02/2024
*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "CPathNode.h"
#include "CPathVolume.h"
#include "CPathVolumeGroundPrio.h"
#include "CPathFindPath.h"
#include "CPathDynamicObstacle.h"

#include "AI_Fotch.generated.h"

UCLASS()
class HB_API AAI_Fotch : public APawn
{
	GENERATED_BODY()

public:
	AAI_Fotch();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Variables", DisplayName = "PathBuffer")
	TArray<FCPathNode> PathBuffer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Property", DisplayName = "Minimum_Fly_Height", meta = (ClampMin = 0))
	float MinFlyHeight = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Property", DisplayName = "Maximum_Fly_Height", meta = (ClampMin = 0))
	float MaxFlyHeight = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Variables", DisplayName = "VolumeRef")
	class ACPathVolume* VolumeRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Variables", DisplayName = "GoToRunning")
	bool GoToRunning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Variables", DisplayName = "CurrentNodeIndex")
	int CurrentNodeIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Variables", DisplayName = "DrawDebugPath")
	bool DrawDebugPath = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Variables", DisplayName = "CurrentInputVector")
	FVector CurrentInputVector = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Variables", DisplayName = "GroundPriority")
	bool UseGroundPriority = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Variables", DisplayName = "UseRandomPath")
	bool UseRandomPath = false;

	FVector Center, Extend;
protected:
	virtual void BeginPlay() override; 

public:

	// Spring Arm Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArmComponent;

	// Camera Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	// Skeletal Mesh Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* SkeletalMeshComponent;

	// Capsule Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCapsuleComponent* CapsuleComponent;

	// Floating Pawn Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", DisplayName = "Flying Pawn Movement")
	class UFloatingPawnMovement* FloatingPawnMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
	UCapsuleComponent* CapsuleComponentVolume;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	virtual void Tick(float DeltaTime) override;

	// Function CheckLocation
	UFUNCTION(BlueprintCallable, Category = "Custom|Functions", DisplayName = "CheckLocation")
	FVector CheckLocation(FVector location);

	// Function GetFurthestPoint
	UFUNCTION(BlueprintCallable, Category = "Custom|Functions", DisplayName = "GetFurthestPoint")
	FVector GetFurthestPoint(FVector location);

	// Function GetRandomPoint
	UFUNCTION(BlueprintCallable, Category = "Custom|Functions", DisplayName = "GetRandomPoint")
	FVector GetRandomPoint();
};
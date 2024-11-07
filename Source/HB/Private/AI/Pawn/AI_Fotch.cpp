// Fill out your copyright notice in the Description page of Project Settings.

/**
* @file AI_Fotch.cpp
* @brief
*
* @author HitBoxTM
*
* @copyright (c) HitBoxTM 2024
* @date 02/11/2023
* @version 1.0
*/
#include "AI/Pawn/AI_Fotch.h"
#include "HB/HB.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "NavigationSystem.h"


#include "Character/HBCharacter.h"
#include "Kismet/GameplayStatics.h"



AAI_Fotch::AAI_Fotch()
{
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("FloatingPawnMovement");

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
    SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
    CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
    CapsuleComponentVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponentVolume"));

	SetRootComponent(CapsuleComponent);
    SkeletalMeshComponent->SetupAttachment(RootComponent);
    SpringArmComponent->SetupAttachment(RootComponent);
	CameraComponent->SetupAttachment(SpringArmComponent);
    CapsuleComponentVolume->SetupAttachment(RootComponent);

	FloatingPawnMovement->UpdatedComponent = RootComponent;

    CapsuleComponentVolume->OnComponentBeginOverlap.AddDynamic(this, &AAI_Fotch::OnOverlapBegin);
}

void AAI_Fotch::BeginPlay()
{
    Super::BeginPlay();

    // Setup the volume reference
    switch (UseGroundPriority)
    {
    case true:
        // Setup the volume reference
        VolumeRef = Cast<ACPathVolumeGroundPrio>(UGameplayStatics::GetActorOfClass(GetWorld(), ACPathVolumeGroundPrio::StaticClass()));
        if (VolumeRef == nullptr)
        {
            UE_LOG(HBGame, Warning, TEXT("No volume ground prio found"));
        }
        else
        {
            UE_LOG(HBGame, Display, TEXT("Volume ground prio found"));
        }
        break;
    case false:
        // Setup the volume reference
        VolumeRef = Cast<ACPathVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), ACPathVolume::StaticClass()));
        if (VolumeRef == nullptr)
        {
            UE_LOG(HBGame, Warning, TEXT("No volume found"));
        }
        else
        {
			UE_LOG(HBGame, Display, TEXT("Volume found"));
		}
        break;
    }

}

void AAI_Fotch::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor->GetClass()->IsChildOf(AHBCharacter::StaticClass()))
    {
        UE_LOG(LogTemp, Error, TEXT("Vif catch by player"));
    }
}

void AAI_Fotch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector AAI_Fotch::CheckLocation(FVector location)
{
	FVector newLocation = location;

    if(newLocation.X >= Center.X + Extend.X)
	{
		newLocation.X = Center.X + Extend.X - 200;
	}
	else if(newLocation.X <= Center.X - Extend.X)
	{
		newLocation.X = Center.X - Extend.X + 200;
	}

    if(newLocation.Y >= Center.Y + Extend.Y)
	{
		newLocation.Y = Center.Y + Extend.Y - 200;
	}
	else if(newLocation.Y <= Center.Y - Extend.Y)
	{
		newLocation.Y = Center.Y - Extend.Y + 200;
	}

	if(newLocation.Z < this->MinFlyHeight)
	{
		newLocation.Z = this->MinFlyHeight;
	}
	else if(newLocation.Z > this->MaxFlyHeight)
	{
		newLocation.Z = this->MaxFlyHeight;
	}

	return newLocation;
}


FVector AAI_Fotch::GetFurthestPoint(FVector location)
{
    TArray<AHBCharacter*> Players;

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHBCharacter::StaticClass(), Actors);

    for (AActor* Actor : Actors)
    {
        AHBCharacter* Player = Cast<AHBCharacter>(Actor);
        if (Player)
        {
            Players.Add(Player);
        }
    }

    if (VolumeRef != nullptr)
    {
        VolumeRef->GetActorBounds(false, Center, Extend);
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("GetFurthestPoint : No volume found"));
		return FVector::ZeroVector;
    }

    FVector Min = Center - Extend;
    FVector Max = Center + Extend;

    FVector FurthestPoint = FVector::ZeroVector;
    double DistanceMax = TNumericLimits<float>::Min();

    if(UseRandomPath)
    {   
        FurthestPoint = GetRandomPoint();
    }
    else
    {
        // Get the furthest point from all players // Algo ultra basique
        for (double x = Min.X; x <= Max.X; x += 1000)
        {
            for (double y = Min.Y; y <= Max.Y; y += 1000)
            {
                FVector PointCourant = FVector(x, y , this->MinFlyHeight);

                for (AHBCharacter* Player : Players)
                {
                    double Distance = FVector::DistSquared(PointCourant, Player->GetActorLocation());
                    if (Distance > DistanceMax)
                    {
						FurthestPoint = PointCourant;
						DistanceMax = Distance;
                    }
                }
            }
        }

        // If no player found, return a random point
        if (DistanceMax == TNumericLimits<float>::Min())
		{
            FurthestPoint = GetRandomPoint();
		}
        else
        {
            FNavLocation RandomPoint;
            UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
            if (NavSystem != nullptr)
			{
				if (NavSystem->GetRandomPointInNavigableRadius(FurthestPoint, 1000, RandomPoint))
				{
					FurthestPoint = RandomPoint.Location;
				}
			}
        }
    }

    return this->CheckLocation(FurthestPoint);
}

FVector AAI_Fotch::GetRandomPoint()
{
    FVector RandomPoint = FVector::ZeroVector;
    FNavLocation RandomNav;
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
    if (NavSystem != nullptr)
    {
        NavSystem->GetRandomPoint(RandomNav);
        RandomPoint = RandomNav.Location;
    }
    else
    {
        FVector Min = Center - Extend;
        FVector Max = Center + Extend;

        // Get Random Point on the volume
        RandomPoint = FVector(FMath::RandRange(Min.X, Max.X),
                              FMath::RandRange(Min.Y, Max.Y),
                              FMath::RandRange(MinFlyHeight, MaxFlyHeight)
        );
    }

    return RandomPoint;
}

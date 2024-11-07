#include "Character/HitboxCustomMovement.h"

#include "Character/HBCharacter.h"
#include "Character/HitboxMovement.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
#if WITH_EDITOR
DEFINE_LOG_CATEGORY_STATIC(LogCMovement, Log, All);
#define PRINTLOG(text, ...) UE_LOG(LogCMovement, Warning, TEXT(text), ##__VA_ARGS__);

#define PRINT(DisplayTime, text, ...) \
			if(GEngine) \
			{ \
				FString ToPrint(m_Owner->GetOwner()->HasAuthority() ? "Server : " : "Client : "); \
				ToPrint.Append(FString::Printf(TEXT(text), ##__VA_ARGS__)); \
				GEngine->AddOnScreenDebugMessage(-1, DisplayTime, FColor::Red, ToPrint); \
			}
#else
#define PRINT(DisplayTime, text, ...)
#define PRINTLOG(text, ...)
#endif

#define ECC_Obstacle ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Flying ECollisionChannel::ECC_GameTraceChannel2
}

/************************************************************************/
/* Quickfall															*/
/************************************************************************/

Quickfall::Quickfall(UHitboxMovement* owner /* = nullptr */)
	: DefaultMovement(owner)
{

}

void Quickfall::Update(double Deltatime)
{
	Super::Update(Deltatime);

	const FVector gravity = { 0.f, 0.f, m_Owner->GetGravityZ() * m_Owner->GoesDownMultiplier };
	m_Owner->Velocity = m_Owner->NewFallVelocity(m_Owner->Velocity, gravity, Deltatime);
	const FRotator rotation = m_Owner->GetOwner()->GetActorRotation();
	FHitResult outhit;
	m_Owner->SafeMoveUpdatedComponent(m_Owner->Velocity * Deltatime, rotation, true, outhit);

	if (outhit.bBlockingHit)
	{
		m_Owner->SetPostLandedPhysics(outhit);
	}
}

/************************************************************************/
/* Slide																*/
/************************************************************************/

Slide::Slide(UHitboxMovement* owner)
	: DefaultMovement(owner)
	, CurrentSlideTime(0.0)
{
	SlideDirection = m_Owner->GetOwner()->GetActorForwardVector();

	SlideRotation = FRotationMatrix::MakeFromX(SlideDirection).Rotator();
	SlideRotation.Pitch = 0;
	SlideRotation.Roll = 0;

	m_Owner->Crouch(true);

	FFindFloorResult FloorResult;
	m_Owner->FindFloor(m_Owner->GetActorLocation()
		, FloorResult
		, false);

	FVector dir = FVector::DownVector * FloorResult.FloorDist;
	m_Owner->GetOwner()->SetActorLocation(m_Owner->GetActorLocation() + dir);
}

void Slide::Update(double Deltatime)
{
	Super::Update(Deltatime);

	if (UpdateSlideTime(Deltatime))
	{
		return;
	}

	FVector OwnerLocation = m_Owner->GetActorLocation();
	FVector TargetDirection = SlideDirection * m_Owner->MaxWalkSpeed;

	{
		FFindFloorResult FloorResult;
		m_Owner->FindFloor(m_Owner->GetActorLocation()
			, FloorResult
			, false);

		if (!FloorResult.bBlockingHit)
		{
			ResetMovement();
			return;
		}
		else if (FloorResult.GetDistanceToFloor() > 1.9f)
		{
			TargetDirection +=  FVector(0.f, 0.f, m_Owner->GetGravityZ()) * Deltatime;
		}
	}

	FHitResult Outhit;
	m_Owner->SafeMoveUpdatedComponent(TargetDirection * Deltatime, SlideRotation, true, Outhit);

	if (Outhit.bBlockingHit)
	{
		float StepHeight = Outhit.ImpactPoint.Z - (m_Owner->GetActorLocation().Z - m_Owner->GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		const float MaxStepHeight = m_Owner->MaxStepHeight - 5.f;

		// PRINTLOG("StepHeight : %f, %f", Outhit.ImpactPoint.Z, (m_Owner->GetActorLocation().Z - m_Owner->GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()))

		if (StepHeight > MaxStepHeight)
		{
			ResetMovement();
			if (IsServer())
			{
				m_Owner->GetOwner<AHBCharacter>()->StunPlayer(true);
			}
			return;
		}
		if (StepHeight > 0)
		{
			GetOwner()->AddActorWorldOffset({0., 0., StepHeight});
		}
	}
	m_Owner->Velocity = TargetDirection;
}

void Slide::ResetMovement()
{
	Super::ResetMovement();
	m_Owner->UnCrouch(true);
	m_Owner->ResolveCollision();
}

bool Slide::UpdateSlideTime(double Deltatime)
{
	CurrentSlideTime += Deltatime;
	if (CurrentSlideTime >= m_Owner->SlideTime)
	{
		ResetMovement();
		return true;
	}
	return false;
}

/************************************************************************/
/* Wall Run Horizontal													*/
/************************************************************************/

WallRunHorizontal::WallRunHorizontal(UHitboxMovement* owner /* = nullptr */)
	: DefaultMovement(owner)
{
	
}

void WallRunHorizontal::Update(double Deltatime)
{
	Super::Update(Deltatime);

	const FVector StartLocation = m_Owner->GetOwner()->GetActorLocation();
	const FVector EndLocation = StartLocation + WallDirection * 20;
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	TArray<TObjectPtr<AActor>> ActorToIgnore = { m_Owner->GetOwner() };
	FHitResult Outhit;
	UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), StartLocation, EndLocation, 35.f, 90.f, UEngineTypes::ConvertToTraceType(ECC_Obstacle), false, ActorToIgnore, EDrawDebugTrace::None, Outhit, true);

	if (!Outhit.bBlockingHit)
	{
		ResetMovement();
		return;
	}

	m_Owner->Velocity = Direction * m_Owner->MaxWalkSpeed;
	m_Owner->SafeMoveUpdatedComponent(m_Owner->Velocity * Deltatime, Direction.ToOrientationRotator(), true, Outhit, ETeleportType::TeleportPhysics);

	if (Outhit.bBlockingHit)
	{
		const float CurrentRadius = m_Owner->GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector VectorStep = Outhit.ImpactPoint - m_Owner->GetActorLocation() + WallDirection * CurrentRadius;
		const float DistanceStep = VectorStep.Length();
		if (DistanceStep < CurrentRadius)
		{
			GetOwner()->AddActorWorldOffset(WallDirection * DistanceStep);
		}
		else
		{
			ResetMovement();
			return;
		}
	}
}

void WallRunHorizontal::ProcessDirection(const FVector& A, const FVector& B)
{
	WallDirection = B * -1;
	Direction = FVector::VectorPlaneProject(A, B);
	Direction *= 1000.f;
	Direction.Normalize();
}

/************************************************************************/
/* Wall Run Vertical													*/
/************************************************************************/

WallRunVertical::WallRunVertical(UHitboxMovement* owner /* = nullptr */)
	: DefaultMovement(owner)
{
}

void WallRunVertical::Update(double Deltatime)
{
	Super::Update(Deltatime);

	FVector StartLocation = m_Owner->GetActorLocation();
	FVector EndLocation = StartLocation + WallDirection * 155.f;

	m_Owner->Velocity = FVector::UpVector * m_Owner->MaxWalkSpeed;

	//if (!IsServer())
	//{
		FHitResult WallOuthit;
		GetWorld()->LineTraceSingleByChannel(WallOuthit, StartLocation, EndLocation, ECC_Obstacle);

		if (!WallOuthit.bBlockingHit)
		{
			m_Owner->Client_ChangeCustomMovement(4);
			return;
		}
	//}

	FRotator OwnerRotation = FRotationMatrix::MakeFromX(WallDirection).Rotator();
	OwnerRotation.Pitch = 0;
	OwnerRotation.Roll = 0;

	FHitResult Outhit;
	m_Owner->SafeMoveUpdatedComponent(m_Owner->Velocity * Deltatime, OwnerRotation, true, Outhit, ETeleportType::TeleportPhysics);

	if (Outhit.bBlockingHit)
	{
		// Ici soit je saute soit je step up?
		double Radius = m_Owner->GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius();
		FVector StartCorrectLocation = m_Owner->GetActorLocation() + WallDirection * Radius;

		FVector ImpactPoint = Outhit.ImpactPoint;
		ImpactPoint.Z = 0;
		StartCorrectLocation.Z = 0;

		FVector DistanceStep = ImpactPoint - StartCorrectLocation;

		double distancemaybe = DistanceStep.Length();

		if (distancemaybe <= Radius)
		{
			m_Owner->MoveUpdatedComponent(distancemaybe * WallDirection * -2, OwnerRotation, false, &Outhit, ETeleportType::TeleportPhysics);
		}
		else
		{
			JumpOfWall();
		}
	}
}

void WallRunVertical::JumpOfWall()
{
	ResetMovement();

	m_Owner->SetMovementMode(MOVE_Falling);
	m_Owner->AddImpulse(WallDirection * 100, true);
}

/************************************************************************/
/* Vault																*/
/************************************************************************/

Vault::Vault(UHitboxMovement* owner /* = nullptr */)
	: DefaultMovement(owner)
{
	if (IsValid(m_Owner) && m_Owner->HasBegunPlay())
	{
		// PRINTLOG("%s Vault Create", IsServer() ? *FString("Server") : *FString("Client"));
		CalcVault();
	}
}

Vault::~Vault()
{
}

bool Vault::CalcVault()
{
	FVector StartLocation = m_Owner->GetActorLocation() + (FVector::UpVector * m_Owner->MaxVaultHeight) + (m_Owner->GetOwner()->GetActorForwardVector() * (m_Owner->DetectionDistance / 2.f));
	FVector EndLocation = StartLocation + FVector::DownVector * (m_Owner->MaxVaultHeight * 2.f + m_Owner->VaultErrorThreshold);

	FCollisionShape tmp;
	FVector3f BoxhalfSize(m_Owner->DetectionDistance / 2.f, 36.f / 2.f, 1.f);
	tmp.SetBox(BoxhalfSize);

	FHitResult outhit;
	GetWorld()->SweepSingleByChannel(outhit, StartLocation, EndLocation, m_Owner->GetOwner()->GetActorRotation().Quaternion(), ECC_Obstacle, tmp);

	if (outhit.bBlockingHit && outhit.Distance > 0.f)
	{
		TargetLocation = outhit.ImpactPoint + FVector::UpVector * 90.f + m_Owner->GetOwner()->GetActorForwardVector() * 30.f;
		m_Owner->GetOwner()->SetActorLocation(m_Owner->GetActorLocation() + FVector::UpVector * m_Owner->GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		TargetDir = TargetLocation - m_Owner->GetActorLocation();
		TargetDir.Normalize();

		if (m_Owner->EnPassantMontage)
			m_Owner->GetOwner<AHBCharacter>()->PlayMontage(m_Owner->EnPassantMontage);
	}
	else
	{
		if (m_Owner->IsCustomMovement())
		{
			ResetMovement();
			return false;
		}
	}

	DrawDebugPoint(GetWorld(), TargetLocation, 20.f, FColor::Red, false, 10.f);

	return true;
}

void Vault::Update(double Deltatime)
{
	Super::Update(Deltatime);

	DrawDebugPoint(GetWorld(), m_Owner->GetActorLocation(), 10.f, FColor::Blue, false, Deltatime);

	FRotator TargetRot = FRotationMatrix::MakeFromX(TargetDir).Rotator();
	TargetRot.Pitch = 0;
	TargetRot.Roll = 0;

	m_Owner->Velocity = TargetDir * m_Owner->MaxWalkSpeed;
	// PRINTLOG("%s : TargetLocation : %s, CurrLoc : %s", IsServer() ? *FString("Server") : *FString("Client"), *TargetLocation.ToString(), *m_Owner->GetActorLocation().ToString())

	FHitResult outhit;
	m_Owner->MoveUpdatedComponent(m_Owner->Velocity * Deltatime, TargetRot, true, &outhit);
	if (outhit.bStartPenetrating)
	{
		FVector dir = m_Owner->GetActorLocation() - outhit.ImpactPoint;
		dir.Normalize();
		dir *= outhit.PenetrationDepth;
		GetOwner()->AddActorWorldOffset(dir);
		// PRINTLOG("%s : G Start penet : %s", IsServer() ? *FString("Server") : *FString("Client"), *dir.ToString())
	}
	if (m_Owner->GetActorLocation().Equals(TargetLocation, m_Owner->MaxWalkSpeed * Deltatime))
	{
		FFindFloorResult FloorResult;
		m_Owner->FindFloor(m_Owner->GetActorLocation(), FloorResult, false);
		// PRINTLOG("%s : Floor distance = %f", IsServer() ? *FString("Server") : *FString("Client"), FloorResult.GetDistanceToFloor())
		if (FloorResult.IsWalkableFloor() && FloorResult.GetDistanceToFloor() < 0.1)
		{
			m_Owner->SetPostLandedPhysics(outhit);
			PRINTLOG("I have land");
		}
		else
		{
			ResetMovement();
			PRINTLOG("I just reset the move");
		}
	}
}

#undef PRINTLOG
#undef PRINT
#undef PRINTA

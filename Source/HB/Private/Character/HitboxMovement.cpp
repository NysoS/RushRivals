// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/HitboxMovement.h"

#include "Animation/AnimInstance.h"
#include "Character/HBCharacter.h"
#include "Character/HitboxCamManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

namespace
{
DEFINE_LOG_CATEGORY_STATIC(LogCustomMovement, Log, All);
#define PRINTLOG(text, ...) UE_LOG(LogCustomMovement, Warning, TEXT(text), ##__VA_ARGS__);

#if WITH_EDITOR

#define PRINT(DisplayTime, text, ...) \
if(GEngine) \
{ \
	FString ToPrint(GetOwner()->HasAuthority() ? "Server : " : "Client : "); \
	ToPrint.Append(FString::Printf(TEXT(text), ##__VA_ARGS__)); \
	GEngine->AddOnScreenDebugMessage(-1, DisplayTime, FColor::Red, ToPrint); \
}
#else
#define PRINT(DisplayTime, text, ...)
#endif // WITH_EDITOR

#define ECC_Obstacle ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Flying ECollisionChannel::ECC_GameTraceChannel2
}

UHitboxMovement::UHitboxMovement() :
	m_CustomMovement(std::variant_alternative_t<0, FCustomMovement>(this))
{
	// Partie pour aller chercher les assets pour éviter de devoir les set à chaque fois dans un nouveau BP
	const ConstructorHelpers::FObjectFinder<UAnimMontage> StopRunAnimMontageUAsset(TEXT("/Game/HitBox/Character/Animation/A_Arret_mainChara_Montage"));
	if (StopRunAnimMontageUAsset.Object)
	{
		StopRunAnimMontage = StopRunAnimMontageUAsset.Object;
	}
	const ConstructorHelpers::FObjectFinder<UAnimMontage> VaultBaseAnimMontageUAsset(TEXT("/Game/HitBox/Character/Animation/A_VaultRebordVariation01_mainChara_Montage"));
	if (VaultBaseAnimMontageUAsset.Object)
	{
		EnPassantMontage = VaultBaseAnimMontageUAsset.Object;
	}
}

// Begin ActorComponent
void UHitboxMovement::BeginPlay()
{
	Super::BeginPlay();

	if (TObjectPtr<ACharacter> CharacterPtr = GetOwner<ACharacter>())
	{
		CharacterPtr->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UHitboxMovement::OnCollideToWall);
	}
}

void UHitboxMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (IsWalking())
	{
		FVector LastInput = GetLastInputVector();
		LastInput.Normalize();
		const float DotValue = GetOwner()->GetActorForwardVector().Dot(LastInput);

		if (DotValue < .0f)
		{
			if (CanReStop)
			{
				CanReStop = false;
				if (StopRunAnimMontage)
				{
					GetOwner<AHBCharacter>()->PlayMontage(StopRunAnimMontage);
				}
			}
			return;
		}
		else
		{
			CanReStop = true;
		}
	}
	if (IsCustomMovement())
	{
		std::visit([&, DeltaTime](auto&& arg)
			{
				arg.Update(DeltaTime);
			}, m_CustomMovement);
	}
}
// End ActorComponent

// Begin CharacterMovementComponent
void UHitboxMovement::SetPostLandedPhysics(const FHitResult& Hit)
{
	std::visit([&](auto&& arg)
		{
			arg.ResetMovement();
		}, m_CustomMovement);
	Super::SetPostLandedPhysics(Hit);
	if (GetOwner()->HasAuthority())
	{
		Client_ResetMovement();
	}
}

void UHitboxMovement::Launch(FVector const& LaunchVel)
{
	std::visit([&](auto&& arg)
		{
			arg.ResetMovement();
		}, m_CustomMovement);
	Super::Launch(LaunchVel);
}
// End CharacterMovementComponent

ECustomMovementMode UHitboxMovement::GetCurrentMovement() const
{
	return std::visit([](auto&& arg) ->ECustomMovementMode
	{
		return arg.CurrentMovement();
	}, m_CustomMovement);
}

bool UHitboxMovement::IsCustomMovement() const
{
	return MovementMode == MOVE_Custom;
}

void UHitboxMovement::ChangerCustomMovement(int NewMovementMode)
{
	if (NewMovementMode == 0) { m_CustomMovement = std::variant_alternative_t<0, FCustomMovement>(this); }
	else if (NewMovementMode == 1) { m_CustomMovement = std::variant_alternative_t<1, FCustomMovement>(this); }
	else if (NewMovementMode == 2) { m_CustomMovement = std::variant_alternative_t<2, FCustomMovement>(this); }
	else if (NewMovementMode == 3) { m_CustomMovement = std::variant_alternative_t<3, FCustomMovement>(this); }
	else if (NewMovementMode == 4) { m_CustomMovement = std::variant_alternative_t<4, FCustomMovement>(this); }
}

void UHitboxMovement::Client_ChangeCustomMovement(int NewMovementMode)
{
	ChangerCustomMovement(NewMovementMode);
	Server_ChangeCustomMovement(NewMovementMode);
}

void UHitboxMovement::Server_ChangeCustomMovement_Implementation(int NewMovementMode)
{
	ChangerCustomMovement(NewMovementMode);
	AllClient_ChangeCustomMovement(NewMovementMode);
}

void UHitboxMovement::AllClient_ChangeCustomMovement_Implementation(int NewMovementMode)
{
	ChangerCustomMovement(NewMovementMode);
}

void UHitboxMovement::ResetMovementMode()
{
	ResetJump();
	bIsKeyWallrunPressed = false;
	if (GetOwner()->HasAuthority())
	{
		SetMovementMode(MOVE_Walking);
	}
}

void UHitboxMovement::UnpackNetworkMovementMode(const uint8 ReceivedMode, TEnumAsByte<EMovementMode>& OutMode, uint8& OutCustomMode, TEnumAsByte<EMovementMode>& OutGroundMode) const
{
	Super::UnpackNetworkMovementMode(ReceivedMode, OutMode, OutCustomMode, OutGroundMode);
}

void UHitboxMovement::ApplyNetworkMovementMode(const uint8 ReceivedMode)
{
	Super::ApplyNetworkMovementMode(ReceivedMode);
}

void UHitboxMovement::ResolveCollision()
{
	FHitResult outhit;
	UKismetSystemLibrary::CapsuleTraceSingle
	(GetWorld()
		, GetOwner()->GetActorLocation()
		, GetOwner()->GetActorLocation()
		, GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius()
		, GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		, UEngineTypes::ConvertToTraceType(ECC_Obstacle)
		, false, { GetOwner() }, EDrawDebugTrace::None, outhit, true);
	if (outhit.bStartPenetrating)
	{
		ResolvePenetration(GetPenetrationAdjustment(outhit), outhit, GetOwner()->GetActorRotation());
	}
}

void UHitboxMovement::Client_ResetMovement_Implementation()
{
	if (!bCanJump)
	{
		ResetMovementMode();
	}
}

/************************************************************************/
/* Wall Run																*/
/************************************************************************/

void UHitboxMovement::OnCollideToWall(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bIsKeyWallrunPressed) { return; }

	if (IsCurrentMovement<WallRunVertical>() || IsCurrentMovement<WallRunHorizontal>())
	{
		return;
	}
	Server_StartWallRun(Hit);
	StartWallRun(Hit);

}

void UHitboxMovement::ActivateWallRun(bool bActivate)
{
	bIsKeyWallrunPressed = bActivate;
	if (!bIsKeyWallrunPressed)
	{
		if (IsCurrentMovement<WallRunVertical>() || IsCurrentMovement<WallRunHorizontal>())
		{
			JumpOffWall();
			Server_JumpOffWall();
		}

		return;
	}

	if (IsCustomMovement())
	{
		if (std::get_if<WallRunHorizontal>(&m_CustomMovement) != nullptr || std::get_if<WallRunVertical>(&m_CustomMovement) != nullptr)
		{
			return;
		}
	}

	const float Radius = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float Height = GetCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float CurrentOffset = 5.f;

	const FVector StartLocation = GetOwner()->GetActorLocation() + FVector::UpVector * CurrentOffset;
	const FVector EndLocation = StartLocation + GetOwner()->GetActorForwardVector() * CurrentOffset;
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	FHitResult Outhit;
	UKismetSystemLibrary::CapsuleTraceSingle
		(GetWorld()
		, StartLocation
		, EndLocation
		, Radius - CurrentOffset
		, Height
		, UCollisionProfile::Get()->ConvertToTraceType(ECC_Obstacle)
		, false, { GetOwner() }, EDrawDebugTrace::None, Outhit, true);

	if (Outhit.bBlockingHit)
	{
		Server_StartWallRun(Outhit);
		StartWallRun(Outhit);
	}
}

void UHitboxMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bCanJump);
}

void UHitboxMovement::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UHitboxMovement::StartWallRun(const FHitResult& Hit)
{
	if (MovementMode == EMovementMode::MOVE_Falling || MovementMode == MOVE_Walking)
	{
		FVector ObstacleNormal = Hit.ImpactNormal;
		ObstacleNormal.Z = 0;
		ObstacleNormal.Normalize();

		FVector DirectionVector = GetOwner()->GetActorForwardVector();
		DirectionVector.Z = 0;
		DirectionVector.Normalize();

		double WallDotDirection = FVector::DotProduct(ObstacleNormal, DirectionVector);

		if (FMath::Abs(WallDotDirection) >= ThresholdVerticalWallRun)
		{
			m_CustomMovement = WallRunVertical(this);

			std::get_if<WallRunVertical>(&m_CustomMovement)->WallDirection = ObstacleNormal * -1;

			if (GetOwner()->HasAuthority())
			{
				AllClient_StartWallRun_Vertical(ObstacleNormal);
			}
		}
		else
		{
			m_CustomMovement = WallRunHorizontal(this);

			std::get_if<WallRunHorizontal>(&m_CustomMovement)->ProcessDirection(DirectionVector, ObstacleNormal);

			float TmpDot = FVector::DotProduct(ObstacleNormal, GetOwner()->GetActorRightVector());
			bWallIsLeftSide = TmpDot < 0;

			if (GetOwner()->HasAuthority())
			{
				AllClient_StartWallRun_Horizontal(DirectionVector, ObstacleNormal);
			}
		}
	}
}

void UHitboxMovement::Server_StartWallRun_Implementation(const FHitResult& Outhit)
{
	if (Outhit.bBlockingHit)
	{
		StartWallRun(Outhit);
	}
}

void UHitboxMovement::AllClient_StartWallRun_Horizontal_Implementation(const FVector& Direction, const FVector& Normal)
{
	m_CustomMovement = WallRunHorizontal(this);

	std::get_if<WallRunHorizontal>(&m_CustomMovement)->ProcessDirection(Direction, Normal);

	float TmpDot = FVector::DotProduct(Normal, GetOwner()->GetActorRightVector());
	bWallIsLeftSide = TmpDot < 0;
}

void UHitboxMovement::AllClient_StartWallRun_Vertical_Implementation(const FVector& Normal)
{
	m_CustomMovement = WallRunVertical(this);

	std::get_if<WallRunVertical>(&m_CustomMovement)->WallDirection = Normal * -1;
}

/************************************************************************/
/* Jump																	*/
/************************************************************************/

void UHitboxMovement::MyJump()
{
	GenericJump();
	Server_Jump();
	ActivateWallRun(true);
}

void UHitboxMovement::Server_Jump_Implementation()
{
	GenericJump();
}

void UHitboxMovement::JumpOffWall()
{
	ResetMovementMode();

	SetMovementMode(MOVE_Falling);
	AddImpulse({100, 100, 100}, true);
}

void UHitboxMovement::Server_JumpOffWall_Implementation()
{
	JumpOffWall();
}

void UHitboxMovement::ResetJump()
{
	GetWorld()->GetTimerManager().ClearTimer(GetOwner<AHBCharacter>()->CurrentJumpOffTimer);
	GetOwner<AHBCharacter>()->CurrentJumpOffTimer.Invalidate();

	bCanJump = true;
}

void UHitboxMovement::GenericJump()
{
	if (IsCustomMovement())
	{
		std::visit(Overload{
		[&](typename Quickfall) {},
		[&](typename Slide) {},
		[&](typename WallRunHorizontal)
		{
			JumpOffWall();
		},
		[&](typename WallRunVertical)
		{
			std::get_if<WallRunVertical>(&m_CustomMovement)->JumpOfWall();
		},
		[&](typename Vault) {}
			}, m_CustomMovement);
	}
	else
	{
		if (bCanJump)
		{
			if (IsFalling())
			{
				StopActiveMovement();
				SetMovementMode(MOVE_Walking);
			}
			GetCharacterOwner()->Jump();
			bCanJump = false;
		}
	}
}

/************************************************************************/
/* Slide																*/
/************************************************************************/

void UHitboxMovement::Duck()
{
	if (!IsCustomMovement())
	{
		if (IsFalling())
		{
			m_CustomMovement = Quickfall(this);
			Server_GoesDown();
		}
		else
		{
			m_CustomMovement = Slide(this);
			Server_Slide();
		}
	}
}

bool UHitboxMovement::IsGoesDown() const
{
	return (std::get_if<Quickfall>(&m_CustomMovement) != nullptr && IsCustomMovement());
}

void UHitboxMovement::Server_Slide_Implementation()
{
	m_CustomMovement = Slide(this);

	AllClient_Slide();
}

void UHitboxMovement::AllClient_Slide_Implementation()
{
	m_CustomMovement = Slide(this);
}

void UHitboxMovement::Server_GoesDown_Implementation()
{
	m_CustomMovement = Quickfall(this);
	AllClient_GoesDown();
}

void UHitboxMovement::AllClient_GoesDown_Implementation()
{
	m_CustomMovement = Quickfall(this);
}


#undef PRINTLOG
#undef PRINT

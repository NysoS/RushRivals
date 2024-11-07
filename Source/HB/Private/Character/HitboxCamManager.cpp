// Copyright Pinguin studio

#include "Character/HitboxCamManager.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/HBCharacter.h"
#include "Gameframework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraManager, Log, All);

#define PRINTLOG(text, ...) UE_LOG(LogCameraManager, Warning, TEXT(text), ##__VA_ARGS__);


UCameraManager::UCameraManager() : Super(),
	bIsCameraLock(false),
	BaseFOV(90),
	MinZoom(30),
	MaxZoom(120),
	CameraSpeed(FVector2D(1)),
	MaxLockRotation(60)
{
	bAutoActivate = true;

#if WITH_EDITOR
	CastOwnerToCharacter();
#endif // WITH_EDITOR

}

void UCameraManager::BeginPlay()
{
	Super::BeginPlay();
	if (!IsActive())
		return;

	if (!CastOwnerToCharacter())
	{
		PRINTLOG("La Camera n'a pas pu être récup.")
	}

}

void UCameraManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsCameraLock)
	{
		
	}

	if (!IsActive())
		return;

}

#if WITH_EDITOR
void UCameraManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UCameraManager, BaseFOV))
	{
		if (CharacterPtr)
		{
			CharacterPtr->GetFollowCamera()->SetFieldOfView(BaseFOV);
		}
	}
}
#endif // WITH_EDITOR

void UCameraManager::AddCameraInput(const FVector2D& dir)
{
	if (!IsActive())
		return;

	CameraMovementToAdd += dir;
	UpdateCamera();
}

void UCameraManager::LockCamera(const bool val)
{
	bIsCameraLock = val;
	//PRINTLOG("Camera Lock")
}

void UCameraManager::SetLockAxis(const FVector axis)
{
	LockAxeCamera = axis;
}

bool UCameraManager::CastOwnerToCharacter()
{
	CharacterPtr = GetOwner<AHBCharacter>();
	return CharacterPtr != nullptr;
}

void UCameraManager::UpdateCamera()
{
	if (!CharacterPtr)
		return;

	// Rotation process la rotation ici.
	FRotator TargetRotation = CharacterPtr->GetControlRotation();
	if (bIsCameraLock)
	{
		// TODO ici faut que je fasse en sorte que la cam soit lock around a axis

		FVector tmpForwardVector = TargetRotation.Vector();
		LockCameraX(tmpForwardVector);
		CharacterPtr->GetController<APlayerController>()->PlayerCameraManager->ViewPitchMin = -MaxLockRotation.Y;
		CharacterPtr->GetController<APlayerController>()->PlayerCameraManager->ViewPitchMax = MaxLockRotation.Y;

		PRINTLOG("Pitch : %f", CharacterPtr->GetController<APlayerController>()->GetControlRotation().Pitch)

	}
	// Truc du genre je sais pas encore
	CharacterPtr->AddControllerYawInput(CameraMovementToAdd.X * CameraSpeed.X);
	CharacterPtr->AddControllerPitchInput(CameraMovementToAdd.Y * CameraSpeed.Y);
	CameraMovementToAdd = FVector2D::Zero();
}

void UCameraManager::LockCameraX(FVector ForwardVector)
{
	FVector tmpAxeLock = FVector::CrossProduct(FVector::UpVector, LockAxeCamera);
	ForwardVector.Z = 0;
	ForwardVector.Normalize();
	double tmpDot = tmpAxeLock.Dot(ForwardVector);
	double currentDegree = tmpDot * 90;
	// Lock la camera sur le X
	if (FMath::IsNearlyEqual(currentDegree, -MaxLockRotation.X, 5) && CameraMovementToAdd.X > 0)
	{
		CameraMovementToAdd.X = 0;
	}
	else if (FMath::IsNearlyEqual(currentDegree, MaxLockRotation.X, 5) && CameraMovementToAdd.X < 0)
	{
		CameraMovementToAdd.X = 0;
	}
}

#undef PRINTLOG

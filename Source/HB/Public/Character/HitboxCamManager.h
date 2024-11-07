// Copyright Pinguin studio
#pragma once

#include "Components/ActorComponent.h"
#include "HitboxCamManager.generated.h"

UCLASS(BlueprintType)
class UCameraManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraManager();

	// Begin Actor override
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	// End Actor override

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void AddCameraInput(const FVector2D& dir);

	// Function Lock Camera.
	// Call this function and set false to Unlock camera
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void LockCamera(const bool val = true);

	void SetLockAxis(const FVector axis);

private:

	bool CastOwnerToCharacter();

	void UpdateCamera();

	void LockCameraX(FVector ForwardVector);

private :

	TObjectPtr<class AHBCharacter> CharacterPtr;

	bool bIsCameraLock;

	// Axe sur le quel la camera sera lock.
	FVector LockAxeCamera;

	// Valeur restant à ajouté.
	FVector2D CameraMovementToAdd;

public:

	// FOV de base de la camera
	// oui je l'ai mis ici pour centraliser les valeurs à 1 endroit.
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, ClampMax = 180, UIMin = 0, UIMax = 180), Category = "Camera|FOV")
	float BaseFOV;

	// Valeur min du zoom possible par les mouvements affecter par le changement de FOV;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, ClampMax = 180, UIMin = 0, UIMax = 180), Category = "Camera|FOV")
	float MinZoom;

	// Valeur max du zoom possible par les mouvements affecter par le changement de FOV;
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0, ClampMax = 180, UIMin = 0, UIMax = 180), Category = "Camera|FOV")
	float MaxZoom;

	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 0, ClampMin = 0), Category = "Camera")
	FVector2D CameraSpeed;

	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 0, UIMax = 89, ClampMin = 0, ClampMax = 89), Category = "Camera")
	FVector2D MaxLockRotation;


};
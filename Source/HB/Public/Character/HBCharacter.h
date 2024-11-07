// Copyright Epic Games, Inc. All Rights Reserved.

/**
* @file HBCharacter.h
* @brief
* 
* @author HitBoxTM 
* 
* @copyright (c) HitBoxTM 2024
* @date 26/10/2023
* @version 1.0
*/
#pragma once

#include "CoreMinimal.h"
#include "Core/Character/EOSCharacter.h"
#include "InputActionValue.h"

#include "HBCharacter.generated.h"

class UWidgetComponent;
class UDataTable;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayAnimMontage, USkeletalMeshComponent*, TargetSkel, UAnimMontage*, MontageToPlay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStun, bool, isStun);


UCLASS(config=Game)
class AHBCharacter : public AEOSCharacter
{
	GENERATED_BODY()

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/// <summary>
	/// Function to call in order to play the animation montage trought all the skeleton
	/// </summary>
	/// <param name="montage">Animation montage to play</param>
	void PlayMontage(class UAnimMontage* montage);

	void StopMontage();

	// Begin AActor
	virtual void BeginPlay();
	virtual void Landed(const FHitResult& Hit) override;
	virtual void PostInitProperties() override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostNetReceive() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	// End AActor

	// Being ACharacter
	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;
	// End ACharacter

	void UpdateSkin(const FString& SkinList);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Property|PlayerStatus")
	bool IsPlayerStun() const;

	UFUNCTION(Server, Reliable)
	void StunPlayer(bool isStun = false);

	UFUNCTION(BlueprintCallable)
	void Vault(const FInputActionValue& _value);

	UFUNCTION()
	void TraceToVault();

	UFUNCTION(Server, Reliable)
	void Server_TraceVault();

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void LookAtTarget(const FInputActionValue& _value);

	void GoesDownFunc(const FInputActionValue& _value);

	void MyJump(const FInputActionValue& _value);

	void ActivatWallRun(const FInputActionValue& _value);

	void GrabAction(const FInputActionValue& _value);

	UFUNCTION(BlueprintCallable)
	void CallFinishParty();
	UFUNCTION(Server, Reliable)
	void FinishPartyWithTeamCatcherId(const int32& TeamId);
	UFUNCTION(NetMulticast, Reliable)
	void OnRep_NetMulticastFinishPartyWithTeamCatcherId(const int32& TeamId);

	UFUNCTION()
	// Begin APawn
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End APawn

	UFUNCTION()
	void SetCantJumpOff();

private:

	UFUNCTION(NetMulticast, Reliable)
	void ActivateStun(const bool isStun);
	
	UFUNCTION(Server, Reliable)
	void Server_ActionGrab();

	UFUNCTION(Client, Reliable)
	void Client_ActionGrab();

	void CommonGrab();

#if WITH_EDITORONLY_DATA
	template <typename type>
	void SetSkinWithName(const FName& name)
	{
		if (TObjectPtr<UDataTable> Currenttable = DataSkinTable[type::name])
		{
			FString context;
			if (FDataSkin* currentSkel = Currenttable->FindRow<FDataSkin>(name, context, true))
			{
				if (currentSkel->SkelMesh.LoadSynchronous())
				{
					check(SkeletalList[type::name]);
					SkeletalList[type::name]->SetSkeletalMeshAsset(currentSkel->SkelMesh.Get());
					SkeletalList[type::name]->SetSkeletalMesh(currentSkel->SkelMesh.Get());
				}
			}
		}
	}
#endif // WITH_EDITORONLY_DATA

public:

	AHBCharacter(const FObjectInitializer& objectInitializer);

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UCameraManager> CameraManager;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStun OnPlayerStun;

	UPROPERTY(BlueprintReadWrite, Category = "Property|PlayerStatus")
	bool bIsGrabing = false;

	UFUNCTION()
	void OnRep_SkinList();

	UPROPERTY(ReplicatedUsing = OnRep_SkinList)
	FString SkinListzerialized;

	UPROPERTY(BlueprintReadOnly)
	FVector2D MovementVector;

	UPROPERTY()
	bool bIsAlreadyHit = false;

	/**
	 * Angle entre 0 - 1 pour 0 le saut sera totalement à la vertical
	 * 1 le saut sera en ligne droite.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "Projection Angle", UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1), Category = "Property|Grab/Dive")
	double ProjectionAngle = 0.4;

	/**
	 * Force appliquer à la projection Grab dive
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta = (DisplayName = "Projection Force"), Category = "Property|Grab/Dive")
	double ProjectionForce = 4800.f;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, TObjectPtr<UDataTable>> DataSkinTable;

	FTimerHandle CurrentJumpOffTimer;

protected:

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property|Skin")
	FName TorsoSkin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property|Skin")
	FName BackSkin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property|Skin")
	FName FeetsSkin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property|Skin")
	FName HandSkin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property|Skin")
	FName HatSkin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property|Skin")
	FName HeadSkin;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Property|Skin")
	FName LegsSkin;
#endif // WITH_EDITORONLY_DATA

	UPROPERTY(EditDefaultsOnly, Category = "Necessary")
	TObjectPtr<UPhysicsAsset> PhysToApply;

	UPROPERTY(EditDefaultsOnly, Category = "Property|Stun")
	float StunTimer = 0.5f;

	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 0, ClampMin = 0), Category = "Property")
	float GrabDistance = 1000.f;

	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 0, ClampMin = 0), Category = "Property")
	float GrabRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 0, ClampMin = 0), Category = "Property")
	float CoyoteTime = 0.7f;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement")
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement")
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement")
	class UInputAction* CameraFocusAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Movement")
	class UInputAction* VaultAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
	class UInputAction* InputGoesDown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
	class UInputAction* InputWallRun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Movement")
	class UInputAction* InputGrabAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	class UAnimMontage* RouladeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	float LandingSpeedToRoll = 1600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reference")
	TSubclassOf<APawn> FlyingPawnRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, SaveGame)
	TMap<FString, TObjectPtr<USkeletalMeshComponent>> SkeletalList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Text Render")
	UWidgetComponent* PseudoRender;

private:

	bool bIsPlayerStun = false;
};


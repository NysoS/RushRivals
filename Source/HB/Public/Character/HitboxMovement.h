// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HitboxCustomMovement.h"
#include "HitboxMovement.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartVault);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndVault);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeDirection, bool, IsMirror);

/**
 * Character movement custom for the hit box that will handle all the parcour mechanique
 */
UCLASS()
class HB_API UHitboxMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UHitboxMovement();

	// Begin ActorComponent
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End ActorComponent

	// Begin CharacterMovementComponent
	virtual void SetPostLandedPhysics(const FHitResult& Hit) override;
	virtual void Launch(FVector const& LaunchVel) override;
	// End CharacterMovementComponent

	UFUNCTION(BlueprintCallable)
	ECustomMovementMode GetCurrentMovement() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsCustomMovement() const;

	template <typename type>
	bool IsCurrentMovement() const
	{
		return IsCustomMovement() && std::get_if<type>(&m_CustomMovement) != nullptr;
	}

	void ChangerCustomMovement(int NewMovementMode);

	void Client_ChangeCustomMovement(int NewMovementMode);

	UFUNCTION(Server, Reliable)
	void Server_ChangeCustomMovement(int NewMovementMode);

	UFUNCTION(NetMulticast, Reliable)
	void AllClient_ChangeCustomMovement(int NewMovementMode);

	void ResetMovementMode();

	UFUNCTION(Client, Reliable)
	void Client_ResetMovement();

	virtual void UnpackNetworkMovementMode(const uint8 ReceivedMode, TEnumAsByte<EMovementMode>& OutMode, uint8& OutCustomMode, TEnumAsByte<EMovementMode>& OutGroundMode) const override;
	virtual void ApplyNetworkMovementMode(const uint8 ReceivedMode) override;

	void ResolveCollision();

	/************************************************************************/
	/* Wall Run																*/
	/************************************************************************/

public:

	UFUNCTION()
	void OnCollideToWall(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ActivateWallRun(bool bActivate);

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called after MovementMode has changed. Base implementation does special handling for starting certain modes, then notifies the CharacterOwner. */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

private:
	
	void StartWallRun(const FHitResult& Hit);

	UFUNCTION(Server, Reliable)
	void Server_StartWallRun(const FHitResult& Outhit);

	UFUNCTION(NetMulticast, Reliable)
	void AllClient_StartWallRun_Horizontal(const FVector& Direction, const FVector& Normal);

	UFUNCTION(NetMulticast, Reliable)
	void AllClient_StartWallRun_Vertical(const FVector& Normal);

public:

	UPROPERTY(EditDefaultsOnly, meta = (UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1), Category = "Property|Wallrun")
	float ThresholdVerticalWallRun = 0.2f;

	UPROPERTY(BlueprintReadOnly, Category = "Property|WallrunHorizontal")
	bool bWallIsLeftSide = false;

private:

	bool bIsKeyWallrunPressed = false;

	//---------------------------Horizontal------------------------------------


	//----------------------------Vertical-------------------------------------


	/************************************************************************/
	/* Jump																	*/
	/************************************************************************/

public:

	UFUNCTION()
	void MyJump();

	UFUNCTION(Server, Reliable)
	void Server_Jump();

	void JumpOffWall();

	UFUNCTION(Server, Reliable)
	void Server_JumpOffWall();

	void ResetJump();

private:
	
	void GenericJump();

public:

	UPROPERTY(Replicated)
	bool bCanJump = true;

	/************************************************************************/
	/* Slide																*/
	/************************************************************************/

public:

	UFUNCTION()
	void Duck();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsGoesDown() const;

private:

	UFUNCTION(Server, Reliable)
	void Server_Slide();

	UFUNCTION(NetMulticast, Reliable)
	void AllClient_Slide();

	UFUNCTION(Server, Reliable)
	void Server_GoesDown();

	UFUNCTION(NetMulticast, Reliable)
	void AllClient_GoesDown();

public:

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Slide Time", UIMin = 0, ClampMin = 0), Category = "Property|Slide")
	float SlideTime = 0.5f;

	/************************************************************************/
	/* Chute rapide varible													*/
	/************************************************************************/

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Chute rapide Multiplier", UIMin = 0, ClampMin = 0), Category = "Property|Chute Rapide")
	float GoesDownMultiplier = 2.f;

	/************************************************************************/
	/* Vault																*/
	/************************************************************************/

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Max vault height", UIMin = 0, ClampMin = 0), Category = "Property|Vault")
	float MaxVaultHeight = 250.f;

	// Valeur de qui permet de diff entre l'animation de passement Haut ou l'anim de passement Bas
	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Vault Threshold height", UIMin = 0, ClampMin = 0), Category = "Property|Vault")
	float VaultTresholdToHigh = 280.f;

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Vault distance movement", UIMin = "0", ClampMin = "0", ForceUnits = "cm"), Category = "Property|Vault")
	float DetectionDistance = 1000.f;

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "vault marge d'erreur"), Category = "Property|Vault")
	float VaultErrorThreshold = 70.f;

	UPROPERTY(EditDefaultsOnly, Category = "Property|Animation")
	class UAnimMontage* EnPassantMontage;

	/************************************************************************/
	/* Custom Movement														*/
	/************************************************************************/

public:

	FCustomMovement m_CustomMovement;

private:

	UPROPERTY()
	class UAnimMontage* StopRunAnimMontage;

	bool CanReStop = true;

};

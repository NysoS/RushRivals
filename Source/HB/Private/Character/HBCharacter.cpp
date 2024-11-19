// Copyright Epic Games, Inc. All Rights Reserved.

/**
* @file HBCharacter.cpp
* @brief
*
* @author HitBoxTM
*
* @copyright (c) HitBoxTM 2024
* @date 26/10/2023
* @version 1.0
*/
#include "Character/HBCharacter.h"

#include "Camera/CameraComponent.h"
#include "Character/HitboxCamManager.h"
#include "Character/HitboxMovement.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Core/GameState/EOSGameState.h"
#include "Core/PlayerState/EOSPlayerState.h"
#include "Engine/DataTable.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WidgetComponent.h"
#include "Gamemode/HBPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Teams/EOSTeamInfo.h"
#include "Teams/EOSTeamSubsystem.h"
#include "Widget/Skin/Enum_SkinPart.h"

namespace
{
	DEFINE_LOG_CATEGORY_STATIC(LogHBCharacter, Log, All);

#ifndef PRINTLOG
#define PRINTLOG(x, ...) UE_LOG(LogHBCharacter, Display, TEXT(x), ##__VA_ARGS__)
#endif


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
#endif

#define ECC_Obstacle ECollisionChannel::ECC_GameTraceChannel1
#define ECC_Flying ECollisionChannel::ECC_GameTraceChannel2
}

//////////////////////////////////////////////////////////////////////////
// AHBCharacter

/**
* @fn AHBCharacter
* @brief Default constructor
*/

AHBCharacter::AHBCharacter(const FObjectInitializer& objectInitialaizer)
	: Super(objectInitialaizer.SetDefaultSubobjectClass<UHitboxMovement>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	PseudoRender = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerNameWidget"));
	PseudoRender->SetupAttachment(RootComponent);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CameraManager = CreateDefaultSubobject<UCameraManager>(TEXT("CameraManagerComponent"));

	GetMesh()->SetVisibility(true);
	GetMesh()->SetHiddenInGame(false);

	SkinListLib::SetAllValue(SkeletalList, {nullptr});

	SkeletalList[SkinTorso::name] = GetMesh();

	SkeletalList[SkinBack::name] = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Back"));
	SkeletalList[SkinBack::name]->SetupAttachment(GetMesh());
	SkeletalList[SkinBack::name]->SetLeaderPoseComponent(GetMesh());

	SkeletalList[SkinFeets::name] = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Feets"));
	SkeletalList[SkinFeets::name]->SetupAttachment(GetMesh());
	SkeletalList[SkinFeets::name]->SetLeaderPoseComponent(GetMesh());

	SkeletalList[SkinHand::name] = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hands"));
	SkeletalList[SkinHand::name]->SetupAttachment(GetMesh());
	SkeletalList[SkinHand::name]->SetLeaderPoseComponent(GetMesh());

	SkeletalList[SkinHat::name] = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hat"));
	SkeletalList[SkinHat::name]->SetupAttachment(GetMesh());
	SkeletalList[SkinHat::name]->SetLeaderPoseComponent(GetMesh());

	SkeletalList[SkinHead::name] = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Head"));
	SkeletalList[SkinHead::name]->SetupAttachment(GetMesh());
	SkeletalList[SkinHead::name]->SetLeaderPoseComponent(GetMesh());

	SkeletalList[SkinLegs::name] = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Legs"));
	SkeletalList[SkinLegs::name]->SetupAttachment(GetMesh());
	SkeletalList[SkinLegs::name]->SetLeaderPoseComponent(GetMesh());

	// To tired to set the value in 
	const ConstructorHelpers::FObjectFinder<UInputAction> InputWallRunObject(TEXT("/Game/ThirdPerson/Input/Actions/IA_WallRun"));
	check(InputWallRunObject.Object)
	InputWallRun = InputWallRunObject.Object;
	
	const ConstructorHelpers::FObjectFinder<UAnimMontage> RouladeMontageObject(TEXT("/Game/HitBox/Character/Animation/M_Roulade_mainChara"));
	check(RouladeMontageObject.Object)
	RouladeMontage = RouladeMontageObject.Object;

#if UE_SERVER == 0
	/************************************************************************/
	/* Skin Data Table														*/
	/************************************************************************/
	SkinListLib::LoadFromPath<SkinBack>("/Game/HitBox/DataTableSkin/DT_BackSkin", DataSkinTable);
	SkinListLib::LoadFromPath<SkinFeets>("/Game/HitBox/DataTableSkin/DT_FeetSkin", DataSkinTable);
	SkinListLib::LoadFromPath<SkinHand>("/Game/HitBox/DataTableSkin/DT_HandSkin", DataSkinTable);
	SkinListLib::LoadFromPath<SkinHat>("/Game/HitBox/DataTableSkin/DT_HatSkin", DataSkinTable);
	SkinListLib::LoadFromPath<SkinHead>("/Game/HitBox/DataTableSkin/DT_HeadSkin", DataSkinTable);
	SkinListLib::LoadFromPath<SkinLegs>("/Game/HitBox/DataTableSkin/DT_LegSkin", DataSkinTable);
	SkinListLib::LoadFromPath<SkinTorso>("/Game/HitBox/DataTableSkin/DT_TorsoSkin", DataSkinTable);
#endif

}

void AHBCharacter::OnRep_SkinList()
{
	UpdateSkin(SkinListzerialized);
}

/**
* @fn BeginPlay
* @brief 
*/
void AHBCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (TObjectPtr<AController> tmpController = GetController())
	{
		//Add Input Mapping Context
		if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (TObjectPtr<AHBPlayerState> playerstate = GetPlayerState<AHBPlayerState>())
	{
		playerstate->LoadSkinData();
		SkinListzerialized = playerstate->SkinListSerialized;
		UpdateSkin(SkinListzerialized);
	}
}

void AHBCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	bIsGrabing = false;
	GetCharacterMovement<UHitboxMovement>()->UnCrouch(true);
	const FVector ZVelocity = FVector(0.f, 0., GetVelocity().Z);
	const bool bIsMovingForward = FVector(GetMovementComponent()->GetLastInputVector().X, GetMovementComponent()->GetLastInputVector().Y, 0.0).Length() > 0.f;

	if (bIsMovingForward && LandingSpeedToRoll <= ZVelocity.Length())
	{
		if (RouladeMontage)
		{
			PlayMontage(RouladeMontage);
		}
	}
}

void AHBCharacter::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITORONLY_DATA
	
	SetSkinWithName<SkinHead>(HeadSkin);
	SetSkinWithName<SkinLegs>(LegsSkin);
	SetSkinWithName<SkinTorso>(TorsoSkin);
	SetSkinWithName<SkinHand>(HandSkin);
	SetSkinWithName<SkinFeets>(FeetsSkin);

#endif // WITH_EDITORONLY_DATA

}

void AHBCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (GetCharacterMovement<UHitboxMovement>()->CustomMovementMode == ECustomMovementMode::CMOVE_Slide)
	{
		if (TObjectPtr<AHBCharacter> Ptr = Cast<AHBCharacter>(Other))
		{
			Ptr->StunPlayer(true);
			return;
		}
	}

	if (bIsGrabing)
	{

		if (FlyingPawnRef && Other->GetClass()->IsChildOf(FlyingPawnRef))
		{
			PRINTLOG("End the game!")
			if (bIsAlreadyHit)
				return;

			return;
		}

		StunPlayer(true);
		bIsGrabing = false;
		GetMovementComponent()->StopMovementImmediately();
	}
}

void AHBCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, SkinListzerialized);
}

void AHBCharacter::PostNetReceive()
{
	Super::PostNetReceive();
	if (TObjectPtr<AHBPlayerState> playerstate = GetPlayerState<AHBPlayerState>())
	{
		playerstate->LoadSkinData();
		SkinListzerialized = playerstate->SkinListSerialized;
		UpdateSkin(SkinListzerialized);
	}
}

#if WITH_EDITOR

void AHBCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
#if WITH_EDITORONLY_DATA

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AHBCharacter, HeadSkin))
	{
		SetSkinWithName<SkinHead>(HeadSkin);
	}
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AHBCharacter, LegsSkin))
	{
		SetSkinWithName<SkinLegs>(LegsSkin);
	}
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AHBCharacter, TorsoSkin))
	{
		SetSkinWithName<SkinTorso>(TorsoSkin);
	}
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AHBCharacter, HandSkin))
	{
		SetSkinWithName<SkinHand>(HandSkin);
	}
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AHBCharacter, FeetsSkin))
	{
		SetSkinWithName<SkinFeets>(FeetsSkin);
	}

#endif // WITH_EDITORONLY_DATA
}

#endif // WITH_EDITOR

void AHBCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);
	PRINTLOG("Im walking of the legde")
	GetWorld()->GetTimerManager().SetTimer(CurrentJumpOffTimer, this, &AHBCharacter::SetCantJumpOff, CoyoteTime, false);
}

void AHBCharacter::UpdateSkin(const FString& SkinList)
{
	TMap<FString, FName> SkinListData;
	SkinListLib::UnserializeMap(SkinListData, SkinList);
	for (const auto& [key, value] : SkinListData)
	{
		if (!value.IsNone())
		{
			if (TObjectPtr<UDataTable> CurrentDataTable = DataSkinTable[key])
			{
				FString context;
				TSoftObjectPtr<USkeletalMesh> CurrentSkel = CurrentDataTable->FindRow<FDataSkin>(value, context)->SkelMesh;
				if (CurrentSkel.LoadSynchronous())
				{
					SkeletalList[key]->SetSkeletalMeshAsset(CurrentSkel.Get());
					SkeletalList[key]->SetSkeletalMesh(CurrentSkel.Get());
					continue;
				}
			}
		}
		SkeletalList[key]->SetSkeletalMeshAsset(nullptr);
		SkeletalList[key]->SetSkeletalMesh(nullptr);
	}
}

bool AHBCharacter::IsPlayerStun() const
{
	return bIsPlayerStun;
}

void AHBCharacter::StunPlayer_Implementation(bool isStun)
{
	ActivateStun(isStun);
	if (isStun)
	{
		FTimerHandle CurrentTimer;
		GetWorld()->GetTimerManager().SetTimer(CurrentTimer, [&]() {
			PRINT(2.f, "now that we don't talk")
				StunPlayer(false);
			}
		, StunTimer, false);
	}
}

void AHBCharacter::ActivateStun_Implementation(bool isStun)
{
	bIsPlayerStun = isStun;
	OnPlayerStun.Broadcast(isStun);
}

//////////////////////////////////////////////////////////////////////////
// Input

/**
* @fn SetupPlayerInputComponent
* @brief
* @param PlayerInputComponent : UInputComponent
*/
void AHBCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AHBCharacter::MyJump);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHBCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHBCharacter::Look);

		//CameraLookToTarget
		EnhancedInputComponent->BindAction(CameraFocusAction, ETriggerEvent::Triggered, this, &AHBCharacter::LookAtTarget);

		// Vault
		EnhancedInputComponent->BindAction(VaultAction, ETriggerEvent::Started, this, &AHBCharacter::Vault);

		// GoesDown
		EnhancedInputComponent->BindAction(InputGoesDown, ETriggerEvent::Started, this, &AHBCharacter::GoesDownFunc);

		// ActivateWallRun
		EnhancedInputComponent->BindAction(InputWallRun, ETriggerEvent::Triggered, this, &AHBCharacter::ActivatWallRun);

		// Grab
		EnhancedInputComponent->BindAction(InputGrabAction, ETriggerEvent::Triggered, this, &AHBCharacter::GrabAction);
	}

}

void AHBCharacter::PlayMontage(UAnimMontage* montage)
{
	if (montage)
	{
		if (GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->Montage_Play(montage);
		}
	}
}

void AHBCharacter::StopMontage()
{
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->StopAllMontages(0.01f);
	}
}

/**
* @fn Move
* @brief
* @param Value : FInputActionValue
*/
void AHBCharacter::Move(const FInputActionValue& Value)
{
	if (IsPlayerStun() || bIsGrabing)
	{
		return;
	}
	// input is a Vector2D
	MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

/**
* @fn Look
* @brief 
* @param Value : FInputActionValue
*/
void AHBCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AHBCharacter::LookAtTarget(const FInputActionValue& _value)
{
	// TODO : get le ptr of the Vif for the lock to work
	TSoftObjectPtr<AActor> UfoPtr = nullptr;
	if (UfoPtr)
	{
		FRotator TargetRotation = FRotationMatrix::MakeFromX(UfoPtr->GetActorLocation() - GetActorLocation()).Rotator();
		GetController<APlayerController>()->SetControlRotation(TargetRotation);
	}
}

void AHBCharacter::GoesDownFunc(const FInputActionValue& _value)
{
	if (_value.Get<bool>())
	{
		GetCharacterMovement<UHitboxMovement>()->Duck();
	}
}

void AHBCharacter::MyJump(const FInputActionValue& _value)
{
	if (IsPlayerStun())
		return;
	
	if (_value.Get<bool>())
	{
		GetCharacterMovement<UHitboxMovement>()->MyJump();
	}
	else
	{
		StopJumping();
		GetCharacterMovement<UHitboxMovement>()->ActivateWallRun(false);
	}
}

void AHBCharacter::ActivatWallRun(const FInputActionValue& _value)
{
}

void AHBCharacter::GrabAction(const FInputActionValue& _value)
{
	Client_ActionGrab();
	Server_ActionGrab();
}

void AHBCharacter::CallFinishParty()
{
	AEOSPlayerState* EOSPlayerState = GetPlayerState<AEOSPlayerState>();
	if (EOSPlayerState)
	{
		FinishPartyWithTeamCatcherId(EOSPlayerState->TeamIdN);
	}
}

void AHBCharacter::FinishPartyWithTeamCatcherId_Implementation(const int32& TeamId)
{
	OnRep_NetMulticastFinishPartyWithTeamCatcherId(TeamId);

	if (TObjectPtr<AEOSGameState> CurrentGamemodePtr = Cast<AEOSGameState>(GetWorld()->GetGameState()))
	{
		CurrentGamemodePtr->UpdateCurrentMatchState(EMatchState::EndRound);
	}
}

void AHBCharacter::OnRep_NetMulticastFinishPartyWithTeamCatcherId_Implementation(const int32& TeamId)
{
	UEOSTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UEOSTeamSubsystem>();
	if (!TeamSubsystem)
		return;

	for (auto& [index, TeamInfo] : TeamSubsystem->GetTeams())
	{
		TeamInfo->SetResultMatch(TeamId);
	}
}

void AHBCharacter::Vault(const FInputActionValue& _value)
{
	if (IsPlayerStun())
		return;

	if (_value.Get<bool>())
	{
		Server_TraceVault();
	}
}

void AHBCharacter::TraceToVault()
{
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + GetActorForwardVector() * GrabDistance;
	FHitResult Outhit;
	UKismetSystemLibrary::SphereTraceSingle
		( GetWorld()
		, StartLocation
		, EndLocation
		, GrabRadius
		, UEngineTypes::ConvertToTraceType(ECC_Flying)
		, false, { this }, EDrawDebugTrace::None, Outhit, true);

	if (Outhit.bBlockingHit && IsValid(Outhit.GetActor()) && Outhit.GetActor()->GetClass()->IsChildOf(FlyingPawnRef))
	{
		if (GetOwner()->HasAuthority())
		{
			CallFinishParty();
		}
	}
}

void AHBCharacter::Server_TraceVault_Implementation()
{
	TraceToVault();
}

void AHBCharacter::SetCantJumpOff()
{
	GetCharacterMovement<UHitboxMovement>()->bCanJump = false;
	PRINTLOG("%s : Character jumpn't : %s", HasAuthority() ? *FString("Server") : *FString("Client"), GetCharacterMovement<UHitboxMovement>()->bCanJump ? *FString("true") : *FString("false"))
}

void AHBCharacter::Server_ActionGrab_Implementation()
{
	if (!bIsGrabing)
	{
		CommonGrab();
	}
}

void AHBCharacter::Client_ActionGrab_Implementation()
{
	CommonGrab();
}

void AHBCharacter::CommonGrab()
{
	if (!bIsGrabing)
	{
		StopMontage();
		bIsGrabing = true;
	}
}

#undef PRINTLOG
#undef PRINT
#undef PRINTA

#pragma once

UENUM(BlueprintType)
enum ECustomMovementMode
{
	// TODO ajouter le reste des mouvements possible.
	CMOVE_None UMETA(DisplayName = "None"),
	CMOVE_Crouch UMETA(DisplayName = "Crouch"),
	CMOVE_AirCrouch UMETA(DisplayName = "Air Crouch"),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_AirSlide UMETA(DisplayName = "Air Slide"),
	CMOVE_WallRunHorizontal UMETA(DisplayName = "Wall Run horizontal"),
	CMOVE_WallRunVertical UMETA(DisplayName = "Wall Run Vertical"),
	CMOVE_Vault UMETA(DisplayName = "Vault"),
	CMOVE_Quickfall UMETA(DisplayName = "Quick fall"),
	CMOVE_Max UMETA(Hidden)
};
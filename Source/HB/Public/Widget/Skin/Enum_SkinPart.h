#pragma once

#include "Engine/DataTable.h"
#include "Enum_SkinPart.generated.h"

UENUM(BlueprintType)
enum ESkinPart
{
	Head,
	Back,
	Legs,
	Torso,
	Foots,
	Gloves,
	Vfx,
	Emotes,
};


USTRUCT(BlueprintType)
struct FDataSkin : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SkinDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<class USkeletalMesh> SkelMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<class UTexture2D> SkinPic;

};

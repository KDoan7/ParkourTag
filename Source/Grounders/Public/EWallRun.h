#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

UENUM()
enum EWallRunSide
{
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UENUM()
enum EWallRunEndReason
{
	FallOffWall UMETA(DisplayName = "FallOffWall"),
	JumpedOffWall UMETA(DisplayName = "JumpedOffWall")
};
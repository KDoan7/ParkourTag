#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

UENUM()
enum EMovementStates
{
	Walking UMETA(DisplayName = "Walking"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Crouching UMETA(DisplayName = "Crouching"),
	Sliding UMETA(DisplayName = "Sliding"),
	WallClimbing UMETA(DisplayName = "WallClimbing"),
	WallRunning UMETA(DisplayName = "WallRunning")
};
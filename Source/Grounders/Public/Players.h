 // Fill out your copyright notice in the Description page of Project Settings.

 #pragma once

 #include "CoreMinimal.h"
 #include "GameFramework/Character.h"
 #include "GameFramework/PlayerController.h"
 #include "GameFramework/SpringArmComponent.h"
 #include "Curves/CurveFloat.h"
 #include "Kismet/GameplayStatics.h"
 #include "Kismet/KismetMathLibrary.h"
 #include "EMovementState.h"
 #include "EWallRun.h"
 #include "Camera/CameraComponent.h"
 #include "Components/TimelineComponent.h"
 #include "Math/Vector.h"
 #include "Math/UnrealMathUtility.h"
 #include "InputCoreTypes.h"
 #include "Players.generated.h"

 class UCapsuleComponent;
 class USphereComponent;
 class UCurveFloat;


 UCLASS()
 class GROUNDERS_API APlayers : public ACharacter
 {
 	GENERATED_BODY()

 public:
 	// Sets default values for this character's properties
 	APlayers();

    DECLARE_EVENT(APlayers, BeginWallRun)
    BeginWallRun& WallRunInitializer() { return BeginWallRunFunctions; }
    BeginWallRun BeginWallRunFunctions;

    DECLARE_EVENT(APlayers, EndWallRun)
    EndWallRun& EndWallRunInitializer() { return EndWallRunReason; }
    EndWallRun EndWallRunReason;

    DECLARE_EVENT(APlayers, BeginWallClimb)
    BeginWallClimb& WallClimbInitializer() { return BeginWallClimbFunctions; }
    BeginWallClimb BeginWallClimbFunctions;

    DECLARE_EVENT(APlayers, EndWallClimb)
    EndWallClimb& EndWallClimbInitializer() { return EndWallClimbFunctions; }
    EndWallClimb EndWallClimbFunctions;

    DECLARE_EVENT(APlayers, BeginSlide)
    BeginSlide& BeginSlideInitializer() { return BeginSlideFunctions; }
    BeginSlide BeginSlideFunctions;

    DECLARE_EVENT(APlayers, EndSlide)
    EndSlide& EndSlideInitializer() { return EndSlideFunctions; }
    EndSlide EndSlideFunctions;

    DECLARE_EVENT(APlayers, BeginCameraTilt)
    BeginCameraTilt& BeginCameraTiltInitializer() { return BeginCameraTiltFunctions; }
    BeginCameraTilt BeginCameraTiltFunctions;

    DECLARE_EVENT(APlayers, EndCameraTilt)
    EndCameraTilt& EndCameraTiltInitializer() { return EndCameraTiltFunctions; }
    EndCameraTilt EndCameraTiltFunctions;

    DECLARE_EVENT(APlayers, BeginCrouch)
    BeginCrouch& BeginCrouchInitializer() { return BeginCrouchFunctions; }
    BeginCrouch BeginCrouchFunctions;

    DECLARE_EVENT(APlayers, EndCrouch)
    EndCrouch& EndCrouchInitializer() { return EndCrouchFunctions; }
    EndCrouch EndCrouchFunctions;

 	UPROPERTY(EditAnywhere)
 		float jumpHeight;

 	UPROPERTY(EditAnywhere)
 		float sprintSpeed;

 	UPROPERTY(EditAnywhere)
 		float sprintIncrease;
	
 	UPROPERTY(EditAnywhere)
 		int maxJumps;

 	UPROPERTY(VisibleAnywhere)
 		int currentJumps;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
 		bool isWallRunning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        bool isSliding;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        bool isCrouching;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
        bool isWallClimbing;
	
 	UPROPERTY(VisibleAnywhere)
 		FVector WallRunDirection;

 	UPROPERTY(BlueprintReadWrite, EditAnywhere)
 		TEnumAsByte<EWallRunSide> wallRunSide;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TEnumAsByte<EMovementStates> currentMovementState;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TEnumAsByte<EMovementStates> prevMovementState;
        
 	UPROPERTY(VisibleAnywhere)
 		FVector rightAxis;

 	UPROPERTY(VisibleAnywhere)
 		FVector forwardAxis;

 	UPROPERTY(BlueprintReadWrite,  Category = "Components")
 		 UCapsuleComponent* HitBox;

    TEnumAsByte<EWallRunEndReason> reasonFellOffWall;

    UPROPERTY(BlueprintReadWRite)
    bool isFacingWall;

    bool CanSlide;
    bool MinSlideLengthReached;
    bool bWallClimbLaunch;
    
    UPROPERTY(EditAnywhere)
        float SlideResetTimer;

    UPROPERTY(EditAnywhere)
        float SlideLengthTimer;

    UPROPERTY(EditAnywhere)
        float MaxSlideSpeed;

    FVector ImpactPoint;
    bool isGrappling;
    float StandingCapsuleHalfHeight;
    float StandingCameraZOffset;
    UPROPERTY(EditAnywhere)
    float walkSpeed;
    UPROPERTY(EditAnywhere)
    float crouchSpeed;
    bool isSprinting;
    UPROPERTY(EditAnywhere)
    float slideSpeed;
    UPROPERTY(EditAnywhere)
    float minSlideLength;
    UPROPERTY(BlueprintReadOnly)
        float wallClimbHeight;
    UPROPERTY(BlueprintReadWrite)
        float currentWallClimbHeight;
    UPROPERTY(BlueprintReadOnly)
        FString prevWallClimbWall;
    UPROPERTY(BlueprintReadOnly)
        FString currentWallClimbWall;

    FTimeline TimeLine;
    FTimeline WallClimbTimeLine;
    FTimeline SlideTimeline;
    FTimeline CameraTiltTimeline;
    FTimeline CrouchTimeline;
    FTimeline WallClimbEndTimeLine;

    UPROPERTY(EditAnywhere, Category = "Timeline")
        UCurveFloat* CurveFloat;

    UPROPERTY(EditAnywhere, Category = "CameraTilt")
        UCurveFloat* CameraTiltCurve;

    UPROPERTY(EditAnywhere, Category = "Crouch")
        UCurveFloat* CrouchCurve;

    UPROPERTY(EditAnywhere, Category = "WallClimb")
        UCurveFloat* WallClimbEndCurve;

 protected:
 	// Called when the game starts or when spawned
 	virtual void BeginPlay() override;

 	void MoveForward(float AxisValue);
 	void MoveRight(float AxisValue);
 	void FBeginCrouch();
 	void FEndCrouch();
    UFUNCTION()
        void FUpdateCrouch();
 	void Jump();
 	const bool CanSurfaceBeWallRan(FVector surfaceNormal);
 	FVector FindLaunchVelocity();
 	const bool AreRequiredKeysDown();
    UFUNCTION()
 	    void UpdateWallRun();
    void FBeginWallRun();
    void FEndWallRun();
    void FBeginSlide();
    UFUNCTION()
        void FUpdateSlide();
    void FEndSlide();
    void QuickTurn();
    void BeginWallClimb();
    void EndWallClimb();
    UFUNCTION()
        void WallClimb();
    FVector CalculateFloorInfluence(FVector FloorNormal);
    UFUNCTION(BlueprintCallable)
        void ResolveMovement();
    void SetMovementState(TEnumAsByte<EMovementStates> NewMovementState);
    void OnMovementStateChanged(TEnumAsByte<EMovementStates> PrevMovementState);
    bool CanStand();
    bool CanSprint();
    void QuitGame();

    //Movement Functions
    void FBeginCameraTilt();
    void FEndCameraTilt();
    UFUNCTION()
        void FUpdateCameraTilt();

    UFUNCTION()
        void FEndWallClimbTimeline();

    void EBeginCrouch();
    void EEndCrouch();
    void BeginSprint();
    void EndSprint();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
        class USpringArmComponent* SpringArmComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
        class UCameraComponent* Camera;

 	const void FindRunDirectionAndSide(FVector wallNormal, TEnumAsByte<EWallRunSide>* outLocalSide, FVector* outReturnVector)
 	{
 		if (FVector2D::DotProduct(FVector2D(wallNormal), FVector2D(GetActorRightVector())) > 0)
 		{
 			*outLocalSide = Left;
 			*outReturnVector = FVector::CrossProduct(wallNormal, FVector(0, 0, 1));
 		}
 		else
 		{
 			*outLocalSide = Right;
 			*outReturnVector = FVector::CrossProduct(wallNormal, FVector(0, 0, -1));
 		};
 	}

 	UFUNCTION()
 		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
 	//virtual void Landed(const FHitResult& Hit) override;

 public:	
 	// Called every frame
 	virtual void Tick(float DeltaTime) override;

 	// Called to bind functionality to input
 	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
 };
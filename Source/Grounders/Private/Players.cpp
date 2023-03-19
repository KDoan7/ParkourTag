// Fill out your copyright notice in the Description page of Project Settings.


#include "Players.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Curves/CurveFloat.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

 //GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Cyan, FString::Printf(TEXT("*IsSliding")));
// Sets default values
APlayers::APlayers()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    sprintSpeed = 2000;
    walkSpeed = 600;
    crouchSpeed = 300;
    wallClimbHeight = 0;
    sprintIncrease = 20;
    slideSpeed = 1.25;
    maxJumps = 2;
    jumpHeight = 2;
    CanSlide = true;

    HitBox = this->GetCapsuleComponent();
    HitBox->OnComponentHit.AddDynamic(this, &APlayers::OnHit);

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(GetMesh(), FName("head"));
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->TargetArmLength = 0;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArmComp);

    //DEFINED IN BP BELOW DOESNT OFFSET BUT DOES ATTACH TO HEAD BONE IT SEEMS
    //SpringArmComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("head"));
    //SpringArmComp->SetWorldLocation(GetMesh()->GetSocketLocation(FName("head")) + FVector(10,0,0));

    this->BeginWallRunFunctions.AddUObject(this, &APlayers::FBeginWallRun);
    this->EndWallRunReason.AddUObject(this, &APlayers::FEndWallRun);

    this->BeginWallClimbFunctions.AddUObject(this, &APlayers::BeginWallClimb);
    this->EndWallClimbFunctions.AddUObject(this, &APlayers::EndWallClimb);

    this->BeginSlideFunctions.AddUObject(this, &APlayers::FBeginSlide);
    this->EndSlideFunctions.AddUObject(this, &APlayers::FEndSlide);

    this->BeginCameraTiltFunctions.AddUObject(this, &APlayers::FBeginCameraTilt);
    this->EndCameraTiltFunctions.AddUObject(this, &APlayers::FEndCameraTilt);

    this->BeginCrouchFunctions.AddUObject(this, &APlayers::EBeginCrouch);
    this->EndCrouchFunctions.AddUObject(this, &APlayers::EEndCrouch);
}

// Called when the game starts or when spawned
void APlayers::BeginPlay()
{
    Super::BeginPlay();

    StandingCapsuleHalfHeight = this->HitBox->GetScaledCapsuleHalfHeight();
    StandingCameraZOffset = Camera->GetRelativeLocation().Z;
    
    FOnTimelineFloat TimelineProgress;
    TimelineProgress.BindUFunction(this, FName("UpdateWallRun"));
    TimeLine.AddInterpFloat(CurveFloat, TimelineProgress);
    TimeLine.SetLooping(true);

    FOnTimelineFloat WallClimbTimeLineProgress;
    WallClimbTimeLineProgress.BindUFunction(this, FName("WallClimb"));
    WallClimbTimeLine.AddInterpFloat(CurveFloat, WallClimbTimeLineProgress);
    WallClimbTimeLine.SetLooping(true);

    FOnTimelineFloat SlideTimeLineProgress;
    SlideTimeLineProgress.BindUFunction(this, FName("FUpdateSlide"));
    SlideTimeline.AddInterpFloat(CurveFloat, SlideTimeLineProgress);
    SlideTimeline.SetLooping(true);

    FOnTimelineFloat CameraTiltTimeLineProgress;
    CameraTiltTimeLineProgress.BindUFunction(this, FName("FUpdateCameraTilt"));
    CameraTiltTimeline.SetTimelineLength(0.2);
    CameraTiltTimeline.AddInterpFloat(CameraTiltCurve, CameraTiltTimeLineProgress);

    FOnTimelineFloat CrouchTimeLineProgress;
    CrouchTimeLineProgress.BindUFunction(this, FName("FUpdateCrouch"));
    CrouchTimeline.SetTimelineLength(0.3);
    CrouchTimeline.AddInterpFloat(CrouchCurve, CrouchTimeLineProgress);

    FOnTimelineFloat WallClimbEndProgress;
    WallClimbEndProgress.BindUFunction(this, FName("FEndWallClimbTimeline"));
    WallClimbEndTimeLine.SetTimelineLength(0.5);
    WallClimbEndTimeLine.AddInterpFloat(WallClimbEndCurve, WallClimbEndProgress);
}

// Called every frame
void APlayers::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeLine.TickTimeline(DeltaTime);
    WallClimbTimeLine.TickTimeline(DeltaTime);
    SlideTimeline.TickTimeline(DeltaTime);
    CameraTiltTimeline.TickTimeline(DeltaTime);
    CrouchTimeline.TickTimeline(DeltaTime);
    WallClimbEndTimeLine.TickTimeline(DeltaTime);


    if (currentMovementState == EMovementStates::Sprinting && GetCharacterMovement()->Velocity.Length() > sprintSpeed)
    {
        if (GetCharacterMovement()->Velocity.Length() < GetCharacterMovement()->MaxWalkSpeed)
        {
            GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->Velocity.Length();
        }  
    }
    else if (currentMovementState == EMovementStates::Sprinting && GetCharacterMovement()->Velocity.Length() < sprintSpeed)
    {
        GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
    }
    if (GetCharacterMovement()->IsMovingOnGround() && currentJumps != 0)
    {
        currentJumps = 0;
    }
    if (currentMovementState == EMovementStates::Crouching && CanStand() || currentMovementState == EMovementStates::Sprinting && GetCharacterMovement()->Velocity.Length() <= walkSpeed) //GetCharacterMovement()->IsMovingOnGround()
    {
        ResolveMovement();
    }
}

// Called to bind functionality to input
void APlayers::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    //Called to bind functionality to input
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APlayers::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APlayers::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &APlayers::Jump);
    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayers::FBeginCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayers::FEndCrouch);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayers::BeginSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayers::EndSprint);
    PlayerInputComponent->BindAction("QuickTurn", IE_Pressed, this, &APlayers::QuickTurn);
    PlayerInputComponent->BindAction("QuitGame", IE_Pressed, this, & APlayers::QuitGame);
}

void APlayers::QuitGame()
{
    //CHANGE THIS TO LOCAL PLAYER
   UKismetSystemLibrary::QuitGame(GetWorld(),UGameplayStatics::GetPlayerController(GetWorld(),0),EQuitPreference::Quit, true);
}

void APlayers::QuickTurn()
{
    SpringArmComp->bEnableCameraRotationLag = true;
    SpringArmComp->CameraRotationLagSpeed = 10;
    //UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetControlRotation(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation() + FRotator(0, 180, 0));
    this->GetController()->SetControlRotation(GetControlRotation() + FRotator(0, 180, 0));
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
        {
            SpringArmComp->bEnableCameraRotationLag = false;
        }, 0.1, false);
}

void APlayers::MoveForward(float AxisValue)
{
    forwardAxis = GetActorForwardVector() * AxisValue;
    AddMovementInput(forwardAxis);
}

void APlayers::MoveRight(float AxisValue)
{
    rightAxis = GetActorRightVector() * AxisValue;
    AddMovementInput(rightAxis);
}

void APlayers::BeginSprint()
{
    isSprinting = true;
    if (currentMovementState == EMovementStates::Walking || currentMovementState == EMovementStates::Crouching)
    {
        ResolveMovement();
    }
}

void APlayers::EndSprint()
{
        isSprinting = false;
    
    if (currentMovementState == EMovementStates::Sprinting)
    {
        ResolveMovement();
    }
}

void APlayers::FBeginCrouch()
{       
    if (!isSliding)
    {
        isCrouching = true;
        //get rid of the dot product stuff if want to be able to slide forward while running backwards again
        if (currentMovementState == EMovementStates::Sprinting && !isSliding && GetCharacterMovement()->Velocity.Length() >= (walkSpeed + 400) && FVector::DotProduct(GetActorForwardVector(), GetCharacterMovement()->Velocity.GetSafeNormal()) > 0)
        {
            SetMovementState(EMovementStates::Sliding);
        }
        else
        {
            //isCrouching = true;
            SetMovementState(EMovementStates::Crouching);
        }
    }
}

void APlayers::EBeginCrouch()
{
    CrouchTimeline.Play();
}

void APlayers::EEndCrouch()
{
    CrouchTimeline.Reverse();
}

void APlayers::FEndCrouch()
{
    isCrouching = false;
}

void APlayers::FUpdateCrouch()
{
        HitBox->SetCapsuleHalfHeight(FMath::GetMappedRangeValueClamped(FVector2D(0, 1), FVector2D(35, StandingCapsuleHalfHeight), CrouchCurve->GetFloatValue(CrouchTimeline.GetPlaybackPosition())));
        if (!GetCharacterMovement()->IsFalling())
        {
            Camera->SetRelativeLocation(FVector(Camera->GetRelativeLocation().X, Camera->GetRelativeLocation().Y, FMath::GetMappedRangeValueClamped(FVector2D(0, 1), FVector2D(-50, StandingCameraZOffset), CrouchCurve->GetFloatValue(CrouchTimeline.GetPlaybackPosition()))));
        }
}


void APlayers::FBeginSlide()
{
    isSliding = true;
    UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetIgnoreMoveInput(true);
    BeginCameraTiltFunctions.Broadcast();
    SlideTimeline.Play();
}

void APlayers::FUpdateSlide()
{
    GetCharacterMovement()->AddForce(CalculateFloorInfluence(GetCharacterMovement()->CurrentFloor.HitResult.Normal));
    if (GetCharacterMovement()->Velocity.Length() < crouchSpeed)
    {
        //ResolveMovement();
        EndSlideFunctions.Broadcast();
        ResolveMovement();
    }
    if (!isCrouching)
    {
        if (GetCharacterMovement()->Velocity.Length() < minSlideLength)
        {
            //ResolveMovement();
            EndSlideFunctions.Broadcast();
            ResolveMovement();
        }
    }
}

void APlayers::FEndSlide()
{
    isSliding = false;
    UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetIgnoreMoveInput(false);
    EndCameraTiltFunctions.Broadcast();
    SlideTimeline.Stop();
}

void APlayers::BeginWallClimb()
{
    isWallClimbing = true;
    currentJumps = 0;
    bWallClimbLaunch = true;
    GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0, 0, 1));
    WallClimbTimeLine.Play();
}

void APlayers::EndWallClimb()
{
    isWallClimbing = false;
    GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0, 0, 0));
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
        {
            bWallClimbLaunch = false;
        }, 0.5, false); 
    WallClimbTimeLine.Stop();
    WallClimbEndTimeLine.PlayFromStart();
    ResolveMovement();
}

void APlayers::WallClimb()
{
    FVector Start = GetActorLocation() + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    FVector End = ((GetActorForwardVector() * 100) + Start);
    FHitResult OutHit;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this->GetOwner());
    //DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);
    bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams);
    
    

    if (isHit)
    {
        //UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetIgnoreMoveInput(true);
        GetCharacterMovement()->Velocity = FVector(0,0,0);
        this->SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + (wallClimbHeight)));
    }
    else
    {
        EndWallClimbFunctions.Broadcast();
        //this->SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2)));
    }
   //UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetIgnoreMoveInput(false);
}

void APlayers::FEndWallClimbTimeline()
{
    this->SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + WallClimbEndCurve->GetFloatValue(WallClimbTimeLine.GetPlaybackPosition() + (HitBox->GetScaledCapsuleHalfHeight() * 2))));
}


void APlayers::Jump()
{
    if (!isWallClimbing)
    {
        if (isWallRunning)
        {
            EndWallRunReason.Broadcast();
        }

        if (GetCharacterMovement()->IsMovingOnGround() || !isWallClimbing)
        {
            wallClimbHeight = 10 + (this->GetVelocity().Length() * 0.01);
        }


        if (currentMovementState != EMovementStates::Sliding)
        {
            if (bWallRunLaunch || isWallClimbing && currentJumps < maxJumps) //isWallRunning
            {
                ACharacter::LaunchCharacter(FindLaunchVelocity(), false, true);
            }
            else if (GetCharacterMovement()->IsFalling() && !isWallRunning && !isWallClimbing && currentJumps < maxJumps)
            {
                currentJumps += 2;
                ACharacter::LaunchCharacter(FVector(FindLaunchVelocity()), false, true);
            }
            if (currentJumps < maxJumps)
            {
                currentJumps++;
                ACharacter::LaunchCharacter(FindLaunchVelocity(), false, true);
            }
        }
    }
}

FVector APlayers::FindLaunchVelocity()
{
    FVector LaunchDirection;

    if (bWallRunLaunch)
    {
        if (wallRunSide == EWallRunSide::Left)
        {
            LaunchDirection = FVector::CrossProduct(FVector(0, 0, 1), (WallRunDirection) * 2);
            return GetCharacterMovement()->JumpZVelocity * LaunchDirection;
        }
        else
        {
            LaunchDirection = FVector::CrossProduct(FVector(0, 0, -1), (WallRunDirection) * 2);
            return GetCharacterMovement()->JumpZVelocity * LaunchDirection;
        }
    }
    if (bWallClimbLaunch)
    {
        LaunchDirection = (this->GetActorForwardVector() * 5) + FVector(0, 0, 1);
        return GetCharacterMovement()->JumpZVelocity * LaunchDirection;
    }
    else if (GetCharacterMovement()->IsFalling())
    {
        //Double jump launch
        LaunchDirection = forwardAxis + rightAxis + FVector(0, 0, 1);

        return GetCharacterMovement()->JumpZVelocity * LaunchDirection;
    }
    else
    {
        LaunchDirection = FVector(0, 0, 1);
        return GetCharacterMovement()->JumpZVelocity * LaunchDirection;
    }
}

void APlayers::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    TEnumAsByte<EWallRunSide> side;
    FVector direction;

    FString teststring = FString::SanitizeFloat(FVector2D::DotProduct(FVector2D(Hit.ImpactNormal), FVector2D(GetActorForwardVector())));



    if (currentMovementState == EMovementStates::Sliding && FVector2D::DotProduct(FVector2D(Hit.ImpactNormal), FVector2D(GetActorForwardVector())) < -0.9 && FVector2D::DotProduct(FVector2D(Hit.ImpactNormal), FVector2D(GetActorForwardVector())) > -1)
    {
        GetCharacterMovement()->Velocity = FVector(0, 0, 0);
    }

    if (FVector2D::DotProduct(FVector2D(Hit.ImpactNormal), FVector2D(GetActorForwardVector())) < -0.9 && FVector2D::DotProduct(FVector2D(Hit.ImpactNormal), FVector2D(GetActorForwardVector())) > -1 && GetCharacterMovement()->IsFalling() && !isCrouching && !isSliding && !isWallClimbing && UGameplayStatics::GetPlayerController(GetWorld(), 0)->IsInputKeyDown(EKeys::W))
    {
        SetMovementState(EMovementStates::WallClimbing);
    }
    

    if (CanSurfaceBeWallRan(Hit.ImpactNormal) && GetCharacterMovement()->IsFalling()  && !isCrouching && !isSliding)
    {
        FindRunDirectionAndSide(Hit.ImpactNormal, &side, &direction);
        WallRunDirection = direction;
        wallRunSide = side;
        if (AreRequiredKeysDown())
        {
            SetMovementState(EMovementStates::WallRunning);
        }
    }
}

void APlayers::FBeginWallRun()
{
    isWallRunning = true;
    bWallRunLaunch = true;
    //GetCharacterMovement()->Velocity.Z = 0;
    if (GetCharacterMovement()->Velocity.Z < 0)
    {
        GetCharacterMovement()->Velocity.Z = GetCharacterMovement()->Velocity.Z * 0.95;
    }

    GetCharacterMovement()->GravityScale = 0;
    GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0, 0, 1));
    BeginCameraTiltFunctions.Broadcast();
    TimeLine.Play();
}

void APlayers::UpdateWallRun()
{
        TEnumAsByte<EWallRunSide> side;
        FVector direction;

        FHitResult OutHit;

        FVector Start = GetActorLocation();
        FVector End;

        FCollisionQueryParams CollisionParams;

        if (isWallClimbing)
        {
            EndWallRunReason.Broadcast();
            SetMovementState(EMovementStates::WallClimbing);
        }

        if (wallRunSide == Left)
        {
            End = Start + FVector::CrossProduct(WallRunDirection, FVector(0, 0, 1)) * 100;
        }
        else
        {
            End = Start + FVector::CrossProduct(WallRunDirection, FVector(0, 0, -1)) * 100;
        }

        CollisionParams.AddIgnoredActor(this->GetOwner());

        // DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
        bool isHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams);
        
        if (isHit && !isFacingWall && !isCrouching && !isSliding)
        {
            FindRunDirectionAndSide(OutHit.ImpactNormal, &side, &direction);

            wallRunSide = side;

            if (side == wallRunSide)
            {
                WallRunDirection.X = direction.X * jumpHeight;
                WallRunDirection.Y = direction.Y * jumpHeight;              
                //WallRunDirection = direction * jumpHeight;
            }
            else
            {
                EndWallRunReason.Broadcast();
            }
        }
        else
        {
            EndWallRunReason.Broadcast();
        }
}

void APlayers::FEndWallRun()
{
    isWallRunning = false;
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
        {
            bWallRunLaunch = false;
            //isWallRunning = false;
        }, 0.1, false);
    GetCharacterMovement()->GravityScale = 1;
    GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0, 0, 0));
    EndCameraTiltFunctions.Broadcast();
    TimeLine.Stop();
    ResolveMovement();
}

const bool APlayers::AreRequiredKeysDown()
{
    if (wallRunSide == Right)
    {
        if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->IsInputKeyDown(EKeys::W) && UGameplayStatics::GetPlayerController(GetWorld(), 0)->IsInputKeyDown(EKeys::D))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (wallRunSide == Left)
    {
        if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->IsInputKeyDown(EKeys::W) && UGameplayStatics::GetPlayerController(GetWorld(), 0)->IsInputKeyDown(EKeys::A))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

const bool APlayers::CanSurfaceBeWallRan(FVector surfaceNormal)
{
    FVector surfaceNormalXY = FVector(surfaceNormal.X, surfaceNormal.Y, 0);
    surfaceNormalXY.Normalize(0.0001);

    if (surfaceNormal.Z < -0.05)
    {
        return false;
    }
    else if (acos(FVector::DotProduct(surfaceNormal, surfaceNormalXY)) < GetCharacterMovement()->GetWalkableFloorAngle())
    {
        return true;
    }
    else
    {
        return false;
    }
}

FVector APlayers::CalculateFloorInfluence(FVector FloorNormal)
{
    if (FloorNormal == this->GetActorUpVector())
    {
        return FVector(0, 0, 0);
    }
    else
    {
        return (FVector::CrossProduct(FloorNormal, FVector::CrossProduct(FloorNormal, this->GetActorUpVector())).GetSafeNormal()) * (FMath::Clamp(FVector::DotProduct(FloorNormal, this->GetActorUpVector()) - 1, 0, 1) * 500000);
    }
}
void APlayers::ResolveMovement()
{
    if (CanSprint())
    {
        SetMovementState(EMovementStates::Sprinting);
    }
    else
    {
        if (CanStand() && !isCrouching)
        {
            SetMovementState(EMovementStates::Walking);
        }
        else
        {
            SetMovementState(EMovementStates::Crouching);
        }
    }
}
void APlayers::SetMovementState(TEnumAsByte<EMovementStates> NewMovementState)
{
        if(NewMovementState != currentMovementState)
        {
        prevMovementState = currentMovementState;
        }       
        currentMovementState = NewMovementState;
        OnMovementStateChanged(prevMovementState);
        switch (currentMovementState)
        {
        case EMovementStates::Sprinting:
            //get rid set timeline length if this causes problems with crouching looking weird. this line fixes rapidly switching between crouching and sprinitng if you sprint backwards and crouch
            CrouchTimeline.SetTimelineLength(0.1);
            EndCrouchFunctions.Broadcast();
            break;
        case EMovementStates::Crouching:
            BeginCrouchFunctions.Broadcast();
            break;
        case EMovementStates::Sliding:
            BeginCrouchFunctions.Broadcast();
            BeginSlideFunctions.Broadcast();
            break;
        case EMovementStates::WallClimbing:
            BeginWallClimbFunctions.Broadcast();
            break;
        case EMovementStates::WallRunning:
            BeginWallRunFunctions.Broadcast();
            break;
        default:
            EndCrouchFunctions.Broadcast();
            break;
        }
}
void APlayers::OnMovementStateChanged(TEnumAsByte<EMovementStates> PrevMovementState)
{
    switch (currentMovementState)
    {
    case EMovementStates::Sprinting:
        if (GetCharacterMovement()->MaxWalkSpeed <= sprintSpeed)
        {
            GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->Velocity.Length();
        }
        break;
    case EMovementStates::Sliding:
        if (GetCharacterMovement()->Velocity.Length() > sprintSpeed)
        {
            GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->Velocity.Length();
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
        }
        break;
    case EMovementStates::WallRunning:
        currentJumps = 0;
        if (GetCharacterMovement()->Velocity.Length() <= sprintSpeed)
        {
            GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->Velocity.Length();
        }
        break;
    case EMovementStates::Crouching:
        GetCharacterMovement()->MaxWalkSpeed = 300;
        break;
    default:
        GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
        break;
    }

    if (prevMovementState == EMovementStates::Sliding)
    {
        GetCharacterMovement()->GroundFriction = 8;
        GetCharacterMovement()->BrakingDecelerationWalking = 2048;
        FEndSlide();
    } 
    if (currentMovementState == EMovementStates::Sliding)
    {
        if (GetCharacterMovement()->Velocity.Length() <  5000.0 && GetCharacterMovement()->Velocity.Length() > walkSpeed)
        {
            GetCharacterMovement()->Velocity = this->GetActorForwardVector() * (GetCharacterMovement()->Velocity.Length() * slideSpeed);
        }
        else
        {
            GetCharacterMovement()->Velocity = this->GetActorForwardVector() * 5000;
        }

        GetCharacterMovement()->GroundFriction = 0;
        GetCharacterMovement()->BrakingDecelerationWalking = 1000;
        minSlideLength = GetCharacterMovement()->Velocity.Length() - 700;
    }
}
bool APlayers::CanStand()
{
    if (isCrouching)
    {
        return false;
    }
    else
    {
        FVector start = this->GetActorLocation() - FVector(0, 0, HitBox->GetScaledCapsuleHalfHeight());
        FVector end = this->GetActorLocation() - FVector(0, 0, HitBox->GetScaledCapsuleHalfHeight()) + FVector(0,0, (StandingCapsuleHalfHeight * 2));
        TArray<FHitResult> OutHit;
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this->GetOwner());
        //DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 1, 0, 1);
        bool isHit = GetWorld()->LineTraceMultiByChannel(OutHit, start, end, ECC_Visibility, CollisionParams);
        if (!isHit)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
bool APlayers::CanSprint()
{
    if (isSprinting && CanStand() || GetCharacterMovement()->Velocity.Length() >= sprintSpeed - 600 && CanStand())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void APlayers::FBeginCameraTilt()
{
    CameraTiltTimeline.Play();
}
void APlayers::FEndCameraTilt()
{
    CameraTiltTimeline.Reverse();
}

void APlayers::FUpdateCameraTilt()
{
    float CameraTiltSide;

    if (wallRunSide == EWallRunSide::Left)
    {
        CameraTiltSide = 1;
    }
    else 
    {
        CameraTiltSide = -1;
    }
    this->GetController()->SetControlRotation(FRotator(this->GetControlRotation().Pitch, this->GetControlRotation().Yaw, CameraTiltCurve->GetFloatValue(CameraTiltTimeline.GetPlaybackPosition()) * CameraTiltSide));
}


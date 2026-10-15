// Fill out your copyright notice in the Description page of Project Settings.


#include "CuteCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

UCuteCharacterMovementComponent::UCuteCharacterMovementComponent()
{
	WalkSpeed = 250.0f;
	MaxWalkSpeed = 500.0f;
	JumpZVelocity = 500.0f;
	AirControl = 0.35f;
	MaxAcceleration = 2048.0f;
	BrakingDecelerationWalking = 2000.0f;
	BrakingDecelerationFalling = 1500.0f;
	MaxFlySpeed = MaxWalkSpeed;
}

void UCuteCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bJumpHeld && IsFalling())
	{
		JumpHeldTime += DeltaTime;
		if (JumpHeldTime >= FlightHoldDuration)
		{
			StartFlight();
		}
	}
	else if (!bJumpHeld)
	{
		JumpHeldTime = 0.0f;
	}

	if (IsMovingOnGround())
	{
		const bool bSprintEligible = ForwardInput > 0.5f && FMath::Abs(RightInput) < 0.2f;
		SprintHeldTime = bSprintEligible ? SprintHeldTime + DeltaTime : 0.0f;
		MaxWalkSpeed = SprintHeldTime >= SprintDelay ? SprintSpeed : WalkSpeed;
	}
	else
	{
		SprintHeldTime = 0.0f;
	}

	UpdateStunnedMesh(DeltaTime);
}

void UCuteCharacterMovementComponent::SetMovementInput(float Right, float Forward)
{
	RightInput = bIsStunned ? 0.0f : FMath::Clamp(Right, -1.0f, 1.0f);
	ForwardInput = bIsStunned ? 0.0f : FMath::Clamp(Forward, -1.0f, 1.0f);
}

void UCuteCharacterMovementComponent::HandleJumpPressed()
{
	bJumpHeld = true;
	JumpHeldTime = 0.0f;

	if (IsFalling())
	{
		StartFlight();
	}
}

void UCuteCharacterMovementComponent::HandleJumpReleased()
{
	bJumpHeld = false;
	JumpHeldTime = 0.0f;
	if (ACharacter* Character = CharacterOwner)
	{
		Character->StopJumping();
	}
}

void UCuteCharacterMovementComponent::SetStunned(bool bNewStunned)
{
	if (bIsStunned == bNewStunned)
	{
		return;
	}

	bIsStunned = bNewStunned;
	if (bIsStunned)
	{
		if (CharacterOwner && CharacterOwner->GetMesh())
		{
			PreStunMeshRelativeRotation = CharacterOwner->GetMesh()->GetRelativeRotation();
			bHasPreStunMeshRotation = true;
		}
		RightInput = 0.0f;
		ForwardInput = 0.0f;
		SprintHeldTime = 0.0f;
		bJumpHeld = false;
	}
	else if (bHasPreStunMeshRotation && CharacterOwner && CharacterOwner->GetMesh())
	{
		CharacterOwner->GetMesh()->SetRelativeRotation(PreStunMeshRelativeRotation);
		bHasPreStunMeshRotation = false;
	}
}

bool UCuteCharacterMovementComponent::IsFlying() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == CuteCustomMovementModeFlight;
}

void UCuteCharacterMovementComponent::StartFlight()
{
	if (!CharacterOwner || IsFlying() || IsMovingOnGround())
	{
		return;
	}

	FlightBaseHeight = UpdatedComponent->GetComponentLocation().Z;
	FlightUpperHeight = FlightBaseHeight + FlightHeight + FlightExtraHeight;
	FlightLowerHeight = FlightBaseHeight + FlightHeight * 0.5f;
	FlightBoostSpeed = 0.0f;
	SetMovementMode(MOVE_Custom, CuteCustomMovementModeFlight);
}

void UCuteCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (IsMovingOnGround())
	{
		JumpHeldTime = 0.0f;
		FlightBoostSpeed = 0.0f;
	}
}

void UCuteCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (CustomMovementMode == CuteCustomMovementModeFlight)
	{
		PhysFlight(DeltaTime, Iterations);
		return;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UCuteCharacterMovementComponent::PhysFlight(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME || !UpdatedComponent || !CharacterOwner)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();
	if (bIsStunned)
	{
		Acceleration = FVector::ZeroVector;
	}

	const float CurrentHeight = UpdatedComponent->GetComponentLocation().Z;
	const bool bHasDirectionalInput = !Acceleration.IsNearlyZero();
	const bool bAboveLowerBand = CurrentHeight > FlightLowerHeight;
	const bool bWantsAscent = bJumpHeld || bHasDirectionalInput;

	if (!bWantsAscent && CurrentHeight <= FlightLowerHeight)
	{
		StopFlight();
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	const bool bWantsForwardBoost = ForwardInput > 0.5f && bAboveLowerBand;
	if (bWantsForwardBoost)
	{
		FlightBoostSpeed = FMath::Min(FlightBoostSpeed + FlightForwardAcceleration * DeltaTime, FMath::Max(0.0f, FlightForwardMaxSpeed - SprintSpeed));
	}
	else
	{
		const float MaxBoostSpeed = FMath::Max(0.0f, FlightForwardMaxSpeed - SprintSpeed);
		FlightBoostSpeed = FMath::Max(0.0f, FlightBoostSpeed - MaxBoostSpeed * DeltaTime / FlightBoostDecayDuration);
	}

	const float PreviousMaxFlySpeed = MaxFlySpeed;
	MaxFlySpeed = bHasDirectionalInput
		? SprintSpeed + FlightBoostSpeed
		: FMath::Max(SprintSpeed + FlightBoostSpeed, Velocity.Size2D());
	CalcVelocity(
		DeltaTime,
		bHasDirectionalInput ? BrakingDecelerationFlying : 0.0f,
		false,
		bHasDirectionalInput ? BrakingDecelerationFlying : 0.0f);
	MaxFlySpeed = PreviousMaxFlySpeed;
	if (!bHasDirectionalInput)
	{
		const float CoastRetention = FMath::Exp2(-DeltaTime / FlightCoastHalfLife);
		Velocity.X *= CoastRetention;
		Velocity.Y *= CoastRetention;
	}

	float TargetVerticalSpeed = 0.0f;
	const float NormalFlightHeight = FlightBaseHeight + FlightHeight;
	if (bJumpHeld)
	{
		if (CurrentHeight < FlightUpperHeight)
		{
			TargetVerticalSpeed = FlightAscentSpeed;
		}
	}
	else if (bHasDirectionalInput)
	{
		if (CurrentHeight < NormalFlightHeight)
		{
			TargetVerticalSpeed = FlightAscentSpeed;
		}
		else if (CurrentHeight > NormalFlightHeight)
		{
			TargetVerticalSpeed = -FlightDescentSpeed;
		}
	}
	else if (CurrentHeight > FlightLowerHeight)
	{
		TargetVerticalSpeed = -FlightDescentSpeed;
	}

	Velocity.Z = FMath::FInterpTo(Velocity.Z, TargetVerticalSpeed, DeltaTime, FlightVerticalAcceleration);

	FVector AdjustedVelocity = Velocity;
	FVector MoveDelta = AdjustedVelocity * DeltaTime;
	if (MoveDelta.Z > 0.0f)
	{
		MoveDelta.Z = FMath::Min(MoveDelta.Z, FMath::Max(0.0f, FlightUpperHeight - CurrentHeight));
	}
	else if (MoveDelta.Z < 0.0f && !bWantsAscent)
	{
		MoveDelta.Z = FMath::Max(MoveDelta.Z, FlightLowerHeight - CurrentHeight);
	}
	FHitResult Hit(1.0f);
	SafeMoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
	if (Hit.IsValidBlockingHit())
	{
		HandleImpact(Hit, DeltaTime, MoveDelta);
		if (Hit.ImpactNormal.Z >= GetWalkableFloorZ())
		{
			SetMovementMode(MOVE_Walking);
			Velocity.Z = 0.0f;
			return;
		}
		SlideAlongSurface(MoveDelta, 1.0f - Hit.Time, Hit.Normal, Hit, true);
	}

	if (UpdatedComponent->GetComponentLocation().Z >= FlightUpperHeight && Velocity.Z > 0.0f)
	{
		Velocity.Z = 0.0f;
	}
}

void UCuteCharacterMovementComponent::StopFlight()
{
	SetMovementMode(MOVE_Falling);
	Velocity.Z = FMath::Min(Velocity.Z, 0.0f);
}

void UCuteCharacterMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	if (bIsStunned && Hit.IsValidBlockingHit() && !Hit.ImpactNormal.IsNearlyZero())
	{
		const float Speed = Velocity.Size();
		const float ImpactAngle = 90.0f - FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(-Velocity.GetSafeNormal(), Hit.ImpactNormal), -1.0f, 1.0f)));
		const float RequiredSpeed = MaxWalkSpeed * WallSlamSpeedMultiplier;
		if (Speed >= RequiredSpeed && ImpactAngle >= WallSlamMinImpactAngle && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("wall slam w.i.p."));
		}
	}

	Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}

void UCuteCharacterMovementComponent::UpdateStunnedMesh(float DeltaTime)
{
	if (!bIsStunned || !CharacterOwner || !CharacterOwner->GetMesh())
	{
		return;
	}

	const FVector FacingDirection = Velocity.Size() >= StunFaceVelocityThreshold
		? -Velocity.GetSafeNormal()
		: FVector::UpVector;
	const FRotator TargetRotation = FRotationMatrix::MakeFromX(FacingDirection).Rotator();
	USkeletalMeshComponent* CharacterMesh = CharacterOwner->GetMesh();
	CharacterMesh->SetWorldRotation(FMath::RInterpTo(CharacterMesh->GetComponentRotation(), TargetRotation, DeltaTime, StunRotationSpeed));
}


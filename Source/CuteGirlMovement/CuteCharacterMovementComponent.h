// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CuteCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class CUTEGIRLMOVEMENT_API UCuteCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCuteCharacterMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

	void SetMovementInput(float Right, float Forward);
	void HandleJumpPressed();
	void HandleJumpReleased();
	void SetStunned(bool bNewStunned);
	bool IsStunned() const { return bIsStunned; }
	bool IsControlling() const { return !FMath::IsNearlyZero(RightInput) || !FMath::IsNearlyZero(ForwardInput); }
	bool IsFlying() const;
	void StartFlight();

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

private:
	enum : uint8
	{
		CuteCustomMovementModeFlight = 1
	};

	void PhysFlight(float DeltaTime, int32 Iterations);
	void UpdateStunnedMesh(float DeltaTime);
	void StopFlight();

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Ground", meta=(ClampMin="0.0"))
	float WalkSpeed = 250.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Ground", meta=(ClampMin="0.0"))
	float SprintSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Ground", meta=(ClampMin="0.0"))
	float SprintDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.0"))
	float FlightHoldDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.0"))
	float FlightHeight = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.0"))
	float FlightExtraHeight = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.0"))
	float FlightAscentSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.0"))
	float FlightDescentSpeed = 160.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.0"))
	float FlightVerticalAcceleration = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.0"))
	float FlightForwardAcceleration = 2400.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.0"))
	float FlightForwardMaxSpeed = 1600.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.01"))
	float FlightCoastHalfLife = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Flight", meta=(ClampMin="0.01"))
	float FlightBoostDecayDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Stun", meta=(ClampMin="0.0"))
	float StunFaceVelocityThreshold = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Stun", meta=(ClampMin="0.0", ClampMax="90.0"))
	float WallSlamMinImpactAngle = 60.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Stun", meta=(ClampMin="0.0"))
	float WallSlamSpeedMultiplier = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Stun", meta=(ClampMin="0.0"))
	float StunRotationSpeed = 8.0f;

	float RightInput = 0.0f;
	float ForwardInput = 0.0f;
	float SprintHeldTime = 0.0f;
	float JumpHeldTime = 0.0f;
	float FlightBoostSpeed = 0.0f;
	float FlightBaseHeight = 0.0f;
	float FlightUpperHeight = 0.0f;
	float FlightLowerHeight = 0.0f;
	bool bJumpHeld = false;
	bool bIsStunned = false;
	FRotator PreStunMeshRelativeRotation = FRotator::ZeroRotator;
	bool bHasPreStunMeshRotation = false;
};

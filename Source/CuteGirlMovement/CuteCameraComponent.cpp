#include "CuteCameraComponent.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

UCuteCameraComponent::UCuteCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsAbsoluteMode = false;
	bEnableSpeedLag = true;
	SpeedLagThreshold = 600.0f;
	SpeedLagMaxSpeed = 1500.0f;
	DefaultInterpSpeed = 15.0f;
	MinLagInterpSpeed = 3.0f;

	bEnableFovSqueeze = true;
	FovSqueezeMaxSpeed = 1800.0f;
	FovSqueezeMinSpeed = 500.0f;
	FovSqueezeAmount = 30.0f;
	FovInterpSpeed = 5.0f;

	ActiveSequence = nullptr;
	CurrentSequenceTime = 0.0f;
	bIsViewInitialized = false;
	ActualFOV = FieldOfView;
}

void UCuteCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	Super::GetCameraView(DeltaTime, DesiredView);

	FVector TargetLocation;
	FRotator TargetRotation;
	float TargetFOV = FieldOfView;

	if (bIsAbsoluteMode)
	{
		TargetLocation = AbsoluteTransform.GetLocation();
		TargetRotation = AbsoluteTransform.Rotator();
	}
	else
	{

		AActor* Owner = GetOwner();
		const float Speed = Owner ? Owner->GetVelocity().Size() : 0.0f;
		const FVector OwnerLocation = Owner ? Owner->GetActorLocation() : GetComponentLocation();
		TargetRotation = GetComponentRotation();

		if (bEnableFovSqueeze && Speed > FovSqueezeMinSpeed)
		{
			const float SpeedRatio = FMath::Clamp(Speed / FMath::Max(FovSqueezeMaxSpeed, 1.0f), 0.0f, 1.0f);
			TargetFOV += FovSqueezeAmount * SpeedRatio;
		}

		if (bEnableSpeedLag)
		{
			float InterpSpeed = DefaultInterpSpeed;
			if (Speed > SpeedLagThreshold)
			{
				const float SpeedFactor = FMath::Clamp(
					(Speed - SpeedLagThreshold) / FMath::Max(SpeedLagMaxSpeed, 1.0f), 0.0f, 1.0f);
				InterpSpeed = FMath::Lerp(DefaultInterpSpeed, MinLagInterpSpeed, SpeedFactor);
			}

			ActualAnchorLocation = bIsViewInitialized
				? FMath::VInterpTo(ActualAnchorLocation, OwnerLocation, DeltaTime, InterpSpeed)
				: OwnerLocation;
		}
		else
		{
			ActualAnchorLocation = OwnerLocation;
		}

		const FVector CameraOffset = GetComponentLocation() - OwnerLocation;
		TargetLocation = ActualAnchorLocation + CameraOffset;
	}

	if (!bIsViewInitialized)
	{
		ActualViewLocation = TargetLocation;
		ActualViewRotation = TargetRotation;
		ActualFOV = TargetFOV;
		bIsViewInitialized = true;
	}
	else
	{
		ActualViewLocation = TargetLocation;
		ActualViewRotation = TargetRotation;
		ActualFOV = FMath::FInterpTo(ActualFOV, TargetFOV, DeltaTime, FovInterpSpeed);
	}

	DesiredView.Location = ActualViewLocation;
	DesiredView.Rotation = ActualViewRotation;
	DesiredView.FOV = ActualFOV;

	if (ActiveSequence)
	{
		CurrentSequenceTime += DeltaTime;
		FVector SeqLoc;
		FRotator SeqRot;
		float SeqFov;
		ActiveSequence->Evaluate(CurrentSequenceTime, SeqLoc, SeqRot, SeqFov);

		if (ActiveSequence->bIsAbsolute)
		{
			DesiredView.Location = SeqLoc;
			DesiredView.Rotation = SeqRot;
			DesiredView.FOV = ActualFOV + SeqFov;
		}
		else
		{
			DesiredView.Location += SeqLoc;
			DesiredView.Rotation += SeqRot;
			DesiredView.FOV += SeqFov;
		}

		if (CurrentSequenceTime >= ActiveSequence->Duration)
		{
			ActiveSequence = nullptr;
		}
	}
}

void UCuteCameraComponent::PlaySequence(UCuteCameraSequence* Sequence)
{
	if (Sequence)
	{
		ActiveSequence = Sequence;
		CurrentSequenceTime = 0.0f;
	}
}

void UCuteCameraComponent::StopSequence()
{
	ActiveSequence = nullptr;
}

void UCuteCameraComponent::ResetInterpolation()
{
	bIsViewInitialized = false;
}
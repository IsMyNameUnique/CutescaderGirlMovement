#include "CuteCameraSequence.h"

void UCuteCameraSequence::Evaluate(float Time, FVector& OutLocation, FRotator& OutRotation, float& OutFov) const
{
	OutLocation = LocationCurve ? LocationCurve->GetVectorValue(Time) : FVector::ZeroVector;
	if (RotationCurve)
	{
		const FVector Rotation = RotationCurve->GetVectorValue(Time);
		OutRotation = FRotator(Rotation.Y, Rotation.Z, Rotation.X);
	}
	else
	{
		OutRotation = FRotator::ZeroRotator;
	}

	OutFov = FovCurve ? FovCurve->GetFloatValue(Time) : 0.0f;
}
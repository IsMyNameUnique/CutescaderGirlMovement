// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "Engine/DataAsset.h"
#include "CuteCameraSequence.generated.h"

/** Curve data for a camera move. */
UCLASS(BlueprintType, Blueprintable)
class CUTEGIRLMOVEMENT_API UCuteCameraSequence : public UDataAsset
{
	GENERATED_BODY()
public:
	// Absolute sequences replace the follow-camera transform.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera Sequence")
	bool bIsAbsolute = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera Sequence")
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera Sequence")
	UCurveVector* LocationCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera Sequence")
	UCurveVector* RotationCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera Sequence")
	UCurveFloat* FovCurve;

	void Evaluate(float Time, FVector& OutLocation, FRotator& OutRotation, float& OutFov) const;

};

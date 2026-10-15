// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "CuteCameraSequence.h"
#include "CuteCameraComponent.generated.h"

/** Third-person camera lag/FOV effects with optional curve-driven sequences. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CUTEGIRLMOVEMENT_API UCuteCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
public:
	UCuteCameraComponent();

	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	UFUNCTION(BlueprintCallable, Category = "Cutescader Camera // Sequence")
	void PlaySequence(UCuteCameraSequence* Sequence);

	UFUNCTION(BlueprintCallable, Category = "Cutescader Camera // Sequence")
	void StopSequence();

	UFUNCTION(BlueprintCallable, Category = "Cutescader Camera // Sequence")
	void ResetInterpolation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // General")
	bool bIsAbsoluteMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // General")
	FTransform AbsoluteTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	bool bEnableSpeedLag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	float SpeedLagThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	float SpeedLagMaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	float DefaultInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	float MinLagInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	bool bEnableFovSqueeze;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	float FovSqueezeMaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	float FovSqueezeMinSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	float FovSqueezeAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutescader Camera // 3rd Person")
	float FovInterpSpeed;

protected:
	UPROPERTY(Transient)
	UCuteCameraSequence* ActiveSequence;

	UPROPERTY(Transient)
	float CurrentSequenceTime;

private:
	FVector ActualAnchorLocation;
	FVector ActualViewLocation;
	FRotator ActualViewRotation;
	float ActualFOV;
	bool bIsViewInitialized;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ActionStateMachine.h"
#include "LocomotionStateMachine.h"
#include "CuteCharacter.generated.h"

class UCuteCameraComponent;
class UCuteCharacterMovementComponent;
class UAbilitySystemComponent;
class UBoxComponent;
class UGameplayAbility;
class UInputAction;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UUserWidget;
struct FInputActionValue;

UCLASS()
class CUTEGIRLMOVEMENT_API ACuteCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACuteCharacter(const FObjectInitializer& ObjectInitializer);
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	void ActivatePrototypeHitbox();
	bool HasHelixAimTarget() const { return bHasHelixAimTarget; }
	FVector GetHelixAimTarget() const { return HelixAimTarget; }
	bool IsHelixSwarmHeld() const { return bHelixSwarmHeld; }

	UFUNCTION(BlueprintCallable, Category="Cute Movement")
	void SetStunned(bool bStunned);

	UFUNCTION(BlueprintCallable, Category="Cute Movement")
	FName GetCurrentLocomotionStateName() const;

	UFUNCTION(BlueprintCallable, Category="Cute Movement")
	FName GetCurrentActionStateName() const;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	void CreateDebugUI();
	void TryActivateHitAbility();
	void StartHelixSwarmInput();
	void StopHelixSwarmInput();
	void UpdateCursorAim(float DeltaTime);
	void SetHelixAimTarget(const FVector& Target);
	void DeactivatePrototypeHitbox();
	UFUNCTION(Server, Reliable)
	void ServerSetHelixSwarmHeld(bool bHeld);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetHelixAimTarget(const FVector_NetQuantize& Target);
	bool ServerSetHelixAimTarget_Validate(const FVector_NetQuantize& Target);
	void ServerSetHelixAimTarget_Implementation(const FVector_NetQuantize& Target);

	UFUNCTION()
	void HandlePrototypeHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetPrototypeHitboxActive(bool bActive);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReportPrototypeHit();

	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);
	virtual void MoveCompleted(const FInputActionValue& Value);
	virtual void JumpStarted();
	virtual void JumpCompleted();

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	TSubclassOf<UUserWidget> DebugUIClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> DebugUIWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Abilities", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> HitboxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Abilities", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> HitboxVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UCuteCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	UInputAction* CuteMoveInputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	UInputAction* CuteLookInputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	UInputAction* CuteMouseLookInputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	UInputAction* CuteJumpInputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Cute Movement|Debug")
	bool bPrintLocomotionStateChanges = true;

	UPROPERTY(EditDefaultsOnly, Category="Abilities|Helix Swarm")
	TSubclassOf<UGameplayAbility> HelixSwarmAbilityClass;

	UPROPERTY(Replicated)
	FVector_NetQuantize HelixAimTarget = FVector::ZeroVector;

	UPROPERTY(Replicated)
	bool bHasHelixAimTarget = false;

private:
	void ServerSetHelixSwarmHeld_Implementation(bool bHeld);

	void UpdateLocomotionState();
	void UpdateActionState();
	UCuteCharacterMovementComponent* GetCuteMovementComponent() const;
	FTimerHandle PrototypeHitboxTimer;
	float CursorAimUpdateAccumulator = 0.0f;
	float LastServerAimUpdateTime = -1.0f;
	bool bHelixSwarmHeld = false;
	bool bPrototypeHitboxActive = false;
	ActionStateMachine ActionStates;
	LocomotionStateMachine LocomotionStates;
};

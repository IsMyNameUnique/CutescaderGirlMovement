// Copyright Epic Games, Inc. All Rights Reserved.

#include "CuteGirlMovementCharacter.h"
#include "Engine/LocalPlayer.h"
#include "CuteCharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "CuteGirlMovement.h"

ACuteGirlMovementCharacter::ACuteGirlMovementCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
}

void ACuteGirlMovementCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		Super::SetupPlayerInputComponent(PlayerInputComponent);
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACuteGirlMovementCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACuteGirlMovementCharacter::DoJumpEnd);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ACuteGirlMovementCharacter::DoJumpEnd);
		}
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACuteGirlMovementCharacter::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACuteGirlMovementCharacter::MoveCompleted);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ACuteGirlMovementCharacter::MoveCompleted);
		}
		if (MouseLookAction)
		{
			EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACuteGirlMovementCharacter::Look);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACuteGirlMovementCharacter::Look);
		}
	}
	else
	{
		UE_LOG(LogCuteGirlMovement, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

}

void ACuteGirlMovementCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ACuteGirlMovementCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ACuteGirlMovementCharacter::DoMove(float Right, float Forward)
{
	UCuteCharacterMovementComponent* Movement = Cast<UCuteCharacterMovementComponent>(GetCharacterMovement());
	if (Movement)
	{
		Movement->SetMovementInput(Right, Forward);
		if (Movement->IsStunned())
		{
			return;
		}
	}

	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ACuteGirlMovementCharacter::DoLook(float Yaw, float Pitch)
{
	const UCuteCharacterMovementComponent* Movement = Cast<UCuteCharacterMovementComponent>(GetCharacterMovement());
	if (GetController() != nullptr && (!Movement || !Movement->IsStunned()))
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ACuteGirlMovementCharacter::DoJumpStart()
{
	JumpStarted();
}

void ACuteGirlMovementCharacter::DoJumpEnd()
{
	JumpCompleted();
}

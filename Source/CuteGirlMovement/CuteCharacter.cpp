// Fill out your copyright notice in the Description page of Project Settings.


#include "CuteCharacter.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CuteCameraComponent.h"
#include "CuteCharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "LocomotionStateMachine.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "PrototypeDebugUI.h"
#include "PrototypeGASAbility.h"
#include "HelixSwarmAbility.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ACuteCharacter::ACuteCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCuteCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	DebugUIClass = UPrototypeDebugUI::StaticClass();
	HelixSwarmAbilityClass = UHelixSwarmAbility::StaticClass();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TranslucentMaterial(TEXT("/Engine/Engine_MI_Shaders/M_Shader_SimpleTranslucent.M_Shader_SimpleTranslucent"));
	HitboxVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HitboxVisual"));
	HitboxVisual->SetupAttachment(RootComponent);
	HitboxVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitboxVisual->SetRelativeLocation(FVector(120.0f, 0.0f, 90.0f));
	HitboxVisual->SetRelativeScale3D(FVector(1.2f));
	HitboxVisual->SetHiddenInGame(true);
	if (CubeMesh.Succeeded())
	{
		HitboxVisual->SetStaticMesh(CubeMesh.Object);
	}
	if (TranslucentMaterial.Succeeded())
	{
		HitboxVisual->SetMaterial(0, TranslucentMaterial.Object);
	}

	HitboxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("HitboxCollision"));
	HitboxCollision->SetupAttachment(RootComponent);
	HitboxCollision->SetBoxExtent(FVector(60.0f));
	HitboxCollision->SetCollisionObjectType(ECC_WorldDynamic);
	HitboxCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitboxCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HitboxCollision->SetGenerateOverlapEvents(true);
	HitboxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitboxCollision->SetRelativeLocation(FVector(120.0f, 0.0f, 90.0f));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCuteCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

}

void ACuteCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	CreateDebugUI();
	HitboxCollision->OnComponentBeginOverlap.AddDynamic(this, &ACuteCharacter::HandlePrototypeHitboxOverlap);
	if (UMaterialInstanceDynamic* Material = HitboxVisual->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.0f, 0.0f));
		Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.0f, 0.0f, 0.0f));
		Material->SetScalarParameterValue(TEXT("Opacity"), 0.35f);
	}

	if (HasAuthority())
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UPrototypeHitAbility::StaticClass(), 1));
		if (HelixSwarmAbilityClass)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(HelixSwarmAbilityClass, 1));
		}
	}
}

UAbilitySystemComponent* ACuteCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACuteCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	CreateDebugUI();
}

void ACuteCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	CreateDebugUI();
}

void ACuteCharacter::CreateDebugUI()
{
	if (!IsLocallyControlled() || DebugUIWidget || !DebugUIClass)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		DebugUIWidget = CreateWidget<UUserWidget>(PlayerController, DebugUIClass);
		if (DebugUIWidget)
		{
			DebugUIWidget->AddToViewport();
		}
	}
}

void ACuteCharacter::TryActivateHitAbility()
{
	const bool bActivated = AbilitySystemComponent
		&& AbilitySystemComponent->TryActivateAbilityByClass(UPrototypeHitAbility::StaticClass());
	if (GetNetMode() != NM_DedicatedServer && GEngine)
	{
		const FString Message = bActivated ? TEXT("Prototype hit activated") : TEXT("Prototype hit unavailable");
		GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()), 1.5f, FColor::Yellow, Message);
	}
}

void ACuteCharacter::ActivatePrototypeHitbox()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PrototypeHitboxTimer);
	MulticastSetPrototypeHitboxActive(true);
	GetWorldTimerManager().SetTimer(PrototypeHitboxTimer, this, &ACuteCharacter::DeactivatePrototypeHitbox, 1.0f, false);
}

void ACuteCharacter::DeactivatePrototypeHitbox()
{
	MulticastSetPrototypeHitboxActive(false);
}

void ACuteCharacter::MulticastSetPrototypeHitboxActive_Implementation(bool bActive)
{
	bPrototypeHitboxActive = bActive;
	HitboxVisual->SetHiddenInGame(!bActive);
	HitboxCollision->SetCollisionEnabled(HasAuthority() && bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ACuteCharacter::HandlePrototypeHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && OtherActor && OtherActor != this && Cast<APawn>(OtherActor))
	{
		MulticastReportPrototypeHit();
	}
}

void ACuteCharacter::MulticastReportPrototypeHit_Implementation()
{
	if (GetNetMode() != NM_DedicatedServer && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("GAS hit!"));
	}
}

void ACuteCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateLocomotionState();
	UpdateCursorAim(DeltaTime);
}

void ACuteCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (CuteMoveInputAction)
		{
			EnhancedInput->BindAction(CuteMoveInputAction, ETriggerEvent::Triggered, this, &ACuteCharacter::Move);
			EnhancedInput->BindAction(CuteMoveInputAction, ETriggerEvent::Completed, this, &ACuteCharacter::MoveCompleted);
			EnhancedInput->BindAction(CuteMoveInputAction, ETriggerEvent::Canceled, this, &ACuteCharacter::MoveCompleted);
		}
		if (CuteLookInputAction)
		{
			EnhancedInput->BindAction(CuteLookInputAction, ETriggerEvent::Triggered, this, &ACuteCharacter::Look);
		}
		if (CuteMouseLookInputAction)
		{
			EnhancedInput->BindAction(CuteMouseLookInputAction, ETriggerEvent::Triggered, this, &ACuteCharacter::Look);
		}
		if (CuteJumpInputAction)
		{
			EnhancedInput->BindAction(CuteJumpInputAction, ETriggerEvent::Started, this, &ACuteCharacter::JumpStarted);
			EnhancedInput->BindAction(CuteJumpInputAction, ETriggerEvent::Completed, this, &ACuteCharacter::JumpCompleted);
			EnhancedInput->BindAction(CuteJumpInputAction, ETriggerEvent::Canceled, this, &ACuteCharacter::JumpCompleted);
		}
	}
	PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ACuteCharacter::TryActivateHitAbility);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACuteCharacter::StartHelixSwarmInput);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ACuteCharacter::StopHelixSwarmInput);
}

void ACuteCharacter::StartHelixSwarmInput()
{
	if (IsLocallyControlled())
	{
		bHelixSwarmHeld = true;
		ServerSetHelixSwarmHeld(true);
	}
}

void ACuteCharacter::StopHelixSwarmInput()
{
	if (IsLocallyControlled())
	{
		bHelixSwarmHeld = false;
		ServerSetHelixSwarmHeld(false);
	}
}

void ACuteCharacter::ServerSetHelixSwarmHeld_Implementation(bool bHeld)
{
	bHelixSwarmHeld = bHeld;

	if (bHeld && AbilitySystemComponent && HelixSwarmAbilityClass)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(HelixSwarmAbilityClass);
	}
}

void ACuteCharacter::UpdateCursorAim(float DeltaTime)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	CursorAimUpdateAccumulator += DeltaTime;
	if (CursorAimUpdateAccumulator < 0.05f)
	{
		return;
	}
	CursorAimUpdateAccumulator = 0.0f;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	// Default to a point far along the player's view direction. This lets the
	// ability fire even when the center-screen trace has no blocking impact.
	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	constexpr float FallbackAimDistance = 50000.0f;
	FVector AimTarget = ViewLocation + ViewRotation.Vector() * FallbackAimDistance;

	FHitResult CursorHit;
	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
	if (ViewportWidth > 0 && ViewportHeight > 0 && PlayerController->GetHitResultAtScreenPosition(
		FVector2D(ViewportWidth * 0.5f, ViewportHeight * 0.5f), ECC_Visibility, true, CursorHit) && CursorHit.bBlockingHit)
	{
		AimTarget = CursorHit.ImpactPoint;
	}

	SetHelixAimTarget(AimTarget);
}

void ACuteCharacter::SetHelixAimTarget(const FVector& Target)
{
	if (HasAuthority())
	{
		HelixAimTarget = Target;
		bHasHelixAimTarget = true;
		ForceNetUpdate();
	}
	else
	{
		ServerSetHelixAimTarget(Target);
	}
}

bool ACuteCharacter::ServerSetHelixAimTarget_Validate(const FVector_NetQuantize& Target)
{
	return !Target.ContainsNaN() && FVector::DistSquared(GetActorLocation(), Target) <= FMath::Square(100000.0f);
}

void ACuteCharacter::ServerSetHelixAimTarget_Implementation(const FVector_NetQuantize& Target)
{
	// Do not discard newer aim samples based on timing. The aim target is also
	// refreshed with a far-away point in the camera direction when the trace misses.
	if (Target.ContainsNaN())
	{
		return;
	}

	HelixAimTarget = Target;
	bHasHelixAimTarget = true;
	ForceNetUpdate();
}

void ACuteCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACuteCharacter, HelixAimTarget);
	DOREPLIFETIME(ACuteCharacter, bHasHelixAimTarget);
}


void ACuteCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (UCuteCharacterMovementComponent* Movement = GetCuteMovementComponent())
	{
		Movement->SetMovementInput(MovementVector.X, MovementVector.Y);
		if (Movement->IsStunned() || !Controller)
		{
			return;
		}
	}
	if (!Controller)
	{
		return;
	}

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), MovementVector.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), MovementVector.X);
}

void ACuteCharacter::Look(const FInputActionValue& Value)
{
	const UCuteCharacterMovementComponent* Movement = GetCuteMovementComponent();
	if (Controller && (!Movement || !Movement->IsStunned()))
	{
		const FVector2D LookVector = Value.Get<FVector2D>();
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
	}
}

void ACuteCharacter::MoveCompleted(const FInputActionValue& Value)
{
	if (UCuteCharacterMovementComponent* Movement = GetCuteMovementComponent())
	{
		Movement->SetMovementInput(0.0f, 0.0f);
	}
}

void ACuteCharacter::JumpStarted()
{
	if (UCuteCharacterMovementComponent* Movement = GetCuteMovementComponent())
	{
		if (Movement->IsStunned())
		{
			return;
		}
		Movement->HandleJumpPressed();
	}
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		Jump();
	}
}

void ACuteCharacter::JumpCompleted()
{
	if (UCuteCharacterMovementComponent* Movement = GetCuteMovementComponent())
	{
		Movement->HandleJumpReleased();
	}
}

void ACuteCharacter::SetStunned(bool bStunned)
{
	if (UCuteCharacterMovementComponent* Movement = GetCuteMovementComponent())
	{
		Movement->SetStunned(bStunned);
	}
}

FName ACuteCharacter::GetCurrentLocomotionStateName() const
{
	return LocomotionStates.GetCurrentStateName();
}

FName ACuteCharacter::GetCurrentActionStateName() const
{
	return ActionStates.GetCurrentStateName();
}

UCuteCharacterMovementComponent* ACuteCharacter::GetCuteMovementComponent() const
{
	return Cast<UCuteCharacterMovementComponent>(GetCharacterMovement());
}

void ACuteCharacter::UpdateLocomotionState()
{
	FCuteLocomotionSnapshot Snapshot;
	Snapshot.bIsGrounded = GetCharacterMovement()->IsMovingOnGround();
	Snapshot.bIsMoving = GetVelocity().SizeSquared() > FMath::Square(3.0f);
	if (const UCuteCharacterMovementComponent* Movement = GetCuteMovementComponent())
	{
		Snapshot.bIsControlling = Movement->IsControlling();
		Snapshot.bIsStunned = Movement->IsStunned();
	}
	const ECuteLocomotionState PreviousState = LocomotionStates.GetCurrentState();
	const ECuteActionState PreviousAction = ActionStates.GetCurrentState();
	const ECuteLocomotionState NewState = LocomotionStates.Update(Snapshot);
	UpdateActionState();
	const ECuteActionState NewAction = ActionStates.GetCurrentState();
	if (bPrintLocomotionStateChanges && (PreviousState != NewState || PreviousAction != NewAction)
		&& GetNetMode() != NM_DedicatedServer && GEngine)
	{
		const FString Message = FString::Printf(
			TEXT("Locomotion: %s | Action: %s"),
			*GetCurrentLocomotionStateName().ToString(),
			*GetCurrentActionStateName().ToString());
		GEngine->AddOnScreenDebugMessage(static_cast<int32>(GetUniqueID()), 2.0f, FColor::Cyan, Message);
	}
}

void ACuteCharacter::UpdateActionState()
{
	FCuteActionSnapshot Snapshot;
	Snapshot.bIsPrototypeHitActive = bPrototypeHitboxActive;
	Snapshot.bIsHelixSwarmActive = bHelixSwarmHeld;
	ActionStates.Update(Snapshot);
}


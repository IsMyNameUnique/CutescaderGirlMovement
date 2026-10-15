#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HelixSphereProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * Server-authoritative helical projectile with locally simulated replicated flight.
 */
UCLASS(Blueprintable, BlueprintType)
class CUTEGIRLMOVEMENT_API AHelixSphereProjectile : public AActor
{
	GENERATED_BODY()

public:
	AHelixSphereProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called by the ability before deferred spawning is finished. */
	void InitializeProjectile(
		float InProjectileSpeed,
		float InProjectileLifetime,
		float InHelixRadius,
		float InHelixAngularSpeed,
		float InHelixPhase);

	/** Prevent this projectile from hitting a specified actor while moving. */
	void IgnoreProjectileCollision(AActor* OtherActor);

	/** Optional Blueprint hook for impact VFX or other impact behavior. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events")
	void BP_OnProjectileImpact(AActor* HitActor, const FHitResult& Hit);

	/** Collision sphere used as the root and gameplay hitbox. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionSphere;

	/** Empty visual slot for a mesh; it has no collision of its own. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Visuals", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* VisualMesh;

	/** Retained for Blueprint compatibility; the helix uses its own movement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Movement", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

protected:
	UFUNCTION()
	void HandleCollisionSphereHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

private:
	UFUNCTION()
	void OnRep_FlightParameters();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastProjectileImpact(AActor* HitActor, FHitResult Hit);

	void ApplyFlightParameters();
	void UpdateFlightLocation();
	float GetServerTime() const;

	UPROPERTY(ReplicatedUsing=OnRep_FlightParameters)
	float FlightSpeed = 1800.0f;

	UPROPERTY(ReplicatedUsing=OnRep_FlightParameters)
	float FlightLifetime = 10.0f;

	UPROPERTY(ReplicatedUsing=OnRep_FlightParameters)
	float HelixRadius = 24.0f;

	UPROPERTY(ReplicatedUsing=OnRep_FlightParameters)
	float HelixAngularSpeed = 18.0f;

	UPROPERTY(ReplicatedUsing=OnRep_FlightParameters)
	float HelixPhase = 0.0f;

	UPROPERTY(ReplicatedUsing=OnRep_FlightParameters)
	FVector FlightOrigin = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing=OnRep_FlightParameters)
	FVector FlightForward = FVector::ForwardVector;

	UPROPERTY(ReplicatedUsing=OnRep_FlightParameters)
	float FlightStartServerTime = 0.0f;

	float FlightTime = 0.0f;
	FVector HelixRight = FVector::RightVector;
	FVector HelixUp = FVector::UpVector;
};

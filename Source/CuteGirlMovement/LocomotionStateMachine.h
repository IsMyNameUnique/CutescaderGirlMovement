#pragma once

#include "CoreMinimal.h"

enum class ECuteLocomotionState : uint8
{
	Ground,
	Moving,
	Air,
	Stunned
};

struct FCuteLocomotionSnapshot
{
	// Inputs captured once per character update.
	bool bIsGrounded = false;
	bool bIsControlling = false;
	bool bIsMoving = false;
	bool bIsStunned = false;
	FName CurrentAction = TEXT("Idle");
};

class CUTEGIRLMOVEMENT_API LocomotionStateMachine
{
public:
	ECuteLocomotionState Update(const FCuteLocomotionSnapshot& Snapshot);
	ECuteLocomotionState GetCurrentState() const { return CurrentState; }
	const FCuteLocomotionSnapshot& GetSnapshot() const { return CurrentSnapshot; }
	FName GetCurrentStateName() const;

private:
	FCuteLocomotionSnapshot CurrentSnapshot;
	ECuteLocomotionState CurrentState = ECuteLocomotionState::Air;
};

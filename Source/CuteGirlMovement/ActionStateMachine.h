#pragma once

#include "CoreMinimal.h"

enum class ECuteActionState : uint8
{
	Idle,
	PrototypeHit,
	HelixSwarm
};

struct FCuteActionSnapshot
{
	// Active action flags; hit takes priority over the held swarm action.
	bool bIsPrototypeHitActive = false;
	bool bIsHelixSwarmActive = false;
};

class CUTEGIRLMOVEMENT_API ActionStateMachine
{
public:
	ECuteActionState Update(const FCuteActionSnapshot& Snapshot);
	ECuteActionState GetCurrentState() const { return CurrentState; }
	FName GetCurrentStateName() const;

	private:
	ECuteActionState CurrentState = ECuteActionState::Idle;
};

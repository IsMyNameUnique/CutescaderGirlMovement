#include "LocomotionStateMachine.h"

ECuteLocomotionState LocomotionStateMachine::Update(const FCuteLocomotionSnapshot& Snapshot)
{
	CurrentSnapshot = Snapshot;
	CurrentState = Snapshot.bIsStunned
		? ECuteLocomotionState::Stunned
		: !Snapshot.bIsGrounded ? ECuteLocomotionState::Air
		: (Snapshot.bIsMoving || Snapshot.bIsControlling) ? ECuteLocomotionState::Moving
		: ECuteLocomotionState::Ground;
	return CurrentState;
}

FName LocomotionStateMachine::GetCurrentStateName() const
{
	switch (CurrentState)
	{
	case ECuteLocomotionState::Moving: return TEXT("Moving");
	case ECuteLocomotionState::Stunned: return TEXT("Stunned");
	case ECuteLocomotionState::Air: return TEXT("Air");
	default: return TEXT("Ground");
	}
}

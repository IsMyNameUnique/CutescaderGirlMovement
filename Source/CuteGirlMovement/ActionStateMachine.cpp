#include "ActionStateMachine.h"

ECuteActionState ActionStateMachine::Update(const FCuteActionSnapshot& Snapshot)
{
	CurrentState = Snapshot.bIsPrototypeHitActive
		? ECuteActionState::PrototypeHit
		: Snapshot.bIsHelixSwarmActive ? ECuteActionState::HelixSwarm : ECuteActionState::Idle;
	return CurrentState;
}

FName ActionStateMachine::GetCurrentStateName() const
{
	switch (CurrentState)
	{
	case ECuteActionState::PrototypeHit: return TEXT("PrototypeHit");
	case ECuteActionState::HelixSwarm: return TEXT("HelixSwarm");
	default: return TEXT("Idle");
	}
}

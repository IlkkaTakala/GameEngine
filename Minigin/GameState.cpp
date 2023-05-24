#include "GameState.h"

using namespace dae;

void dae::GameStateManager::ClearStates()
{
	for (auto& [handle, state] : States) {
		delete state;
	}
	States.clear();
}

void dae::GameStateManager::CheckState()
{
	if (CurrentState == 0) {
		if (!States.empty()) {
			CurrentState = States.begin()->first;
			States[CurrentState]->Init();
		}
		else return;
	}
	for (auto& p : States[CurrentState]->Paths) {
		if (p.second(States[CurrentState])) {
			if (auto it = States.find(CurrentState); it != States.end()) it->second->Exit();
			CurrentState = p.first;
			States[CurrentState]->Init();
		}
	}
}

void dae::GameStateManager::UpdateState(float delta)
{
	if (CurrentState == 0) return;
	States[CurrentState]->Update(delta);
}

StateHandle dae::GameStateManager::AddState(GameState* state)
{
	StateHandle out = high++;
	States.emplace(out, state);
	return out;
}

bool dae::GameStateManager::RemoveState(StateHandle state)
{
	if (auto it = States.find(state); it != States.end()) delete it->second;
	return States.erase(state);
}

StateHandle dae::GameStateManager::GetState(const std::string& name)
{
	for (auto& [handle, state] : States) {
		if (state->name == name) return handle;
	}
	return StateHandle();
}

void dae::GameStateManager::SetState(StateHandle state)
{
	if (auto it = States.find(state); it != States.end()) it->second->Exit();
	CurrentState = state;
	States[CurrentState]->Init();
}

void dae::GameStateManager::AddPath(StateHandle from, StateHandle to, std::function<bool(GameState*)> path)
{
	States[from]->Paths.emplace_back(to, path);
}
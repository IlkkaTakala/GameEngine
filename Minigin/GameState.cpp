#include "GameState.h"

using namespace dae;

void dae::StateManager::ClearStates()
{
	for (auto& [handle, state] : States) {
		delete state;
	}
	States.clear();
}

void dae::StateManager::CheckState()
{
	if (CurrentState == 0) {
		if (!States.empty()) {
			CurrentState = States.begin()->first;
			States[CurrentState]->IsActive = true;
			States[CurrentState]->Init();
		}
		else return;
	}
	for (auto& p : States[CurrentState]->Paths) {
		if (p.second(States[CurrentState])) {
			if (auto it = States.find(CurrentState); it != States.end()) {
				it->second->IsActive = false;
				it->second->Exit();
			}
			CurrentState = p.first;
			States[CurrentState]->IsActive = true;
			States[CurrentState]->Init();
		}
	}
}

void dae::StateManager::UpdateState(float delta)
{
	if (CurrentState == 0) return;
	States[CurrentState]->Update(delta);
}

StateHandle dae::StateManager::AddState(GameState* state)
{
	StateHandle out = high++;
	States.emplace(out, state);
	return out;
}

bool dae::StateManager::RemoveState(StateHandle state)
{
	if (auto it = States.find(state); it != States.end()) delete it->second;
	return States.erase(state);
}

GameState* dae::StateManager::GetState(const std::string& name)
{
	for (auto& [handle, state] : States) {
		if (state->name == name) return state;
	}
	return nullptr;
}

void dae::StateManager::SetState(StateHandle state)
{
	if (auto it = States.find(state); it != States.end()) it->second->Exit();
	CurrentState = state;
	States[CurrentState]->Init();
}

dae::StateManager::~StateManager()
{
	ClearStates();
}

void dae::StateManager::AddPath(StateHandle from, StateHandle to, std::function<bool(GameState*)> path)
{
	States[from]->Paths.emplace_back(to, path);
}
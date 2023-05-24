#pragma once
#include "Singleton.h"
#include <string>
#include <functional>
#include <unordered_map>

namespace dae {

	using StateHandle = size_t;
	class GameState;

class GameStateManager final : public Singleton<GameStateManager>
{
	
public:
	void ClearStates();

	void CheckState();
	void UpdateState(float);

	StateHandle AddState(GameState*);
	bool RemoveState(StateHandle state);
	StateHandle GetState(const std::string& name);

	void AddPath(StateHandle from, StateHandle to, std::function<bool(GameState*)> path);

	void SetState(StateHandle state);

private:

	StateHandle CurrentState{0};
	std::unordered_map<StateHandle, GameState*> States;
	StateHandle high{1};
};

class GameState {
public:
	GameState(const std::string& name) : name(name) {}

	virtual void Update(float) {}
	virtual void Init() = 0;
	virtual void Exit() {}
protected:
	virtual ~GameState() {}
private:
	friend class GameStateManager;
	std::string name;
	std::vector<std::pair<StateHandle, std::function<bool(GameState*)>>> Paths;
};

}

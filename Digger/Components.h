#pragma once
#include "Data.h"
#include "Delegates.h"
#include "Grid.h"
#include "EngineTime.h"
#include <mutex>
#include "GameState.h"

namespace dae {
	class TextComponent;
	class TransformComponent;
	class SpriteComponent;
	class InputComponent;
	class UIComponent;
}
class Grid;

class GridMoveComponent final : public dae::Component<GridMoveComponent>
{

public:

	void OnCreated() override;
	void ComponentUpdate(float /*delta*/) override;
	void Init(float s, Grid* gridC, int x = 0, int y = 0);

	void SetSpeed(float s)
	{
		speed = s;
	}

	void SetCell(int x, int y);

	Direction GetDirection() const { return direction; }

	void SetDirection(Direction dir, bool force = false);

	bool Move(float delta);

	bool IsInProgress() const { return progress < maxProgress&& progress > maxProgress / 2; }

	glm::ivec2 GetGridLoc() const { return grid; }
	Grid* GetGrid() const { return gridComponent.Get(); }

	dae::MulticastDelegate<Direction> OnDirectionChanged;
	dae::MulticastDelegate<Grid*, Direction, int, int> OnGridChanged;
	dae::MulticastDelegate<Grid*, Direction, int, int> OnMoved;
	std::function<bool(Grid*, Direction, int, int)> GetCanMove;

private:
	void OnDestroyed() override;

	void UpdateDirection();

	dae::ComponentRef<dae::TransformComponent> Transform;

	dae::ComponentRef<Grid> gridComponent;
	float* target;
	int* targetGrid;
	glm::ivec2 grid{ 0 };
	glm::vec2 actual{ 0 };
	float progress{ 0.f };
	Direction direction{ 0 };
	int oldDirection{ 0 };
	float speed{ 0.f };
	float maxProgress;
};

class PlayerComponent final : public dae::Component<PlayerComponent>
{
public:

	void Init(dae::User user, const std::string& name, const std::string& sprite) {
		UserID = user;
		Name = name;
		Sprite = sprite;
	}

	bool TakeDamage();
	bool IsAlive() { return Lives > 0; }
	void GiveScore(int amount);

	bool TryFireball();

	void LevelChanged(int x, int y);

	int GetLives() const { return Lives; }
	int GetScore() const { return Score; }
	const std::string& GetName() const { return Name; }
	dae::User GetID() const { return UserID; }

	dae::MulticastDelegate<> OnDeath;
	dae::MulticastDelegate<> OnDamaged;
	dae::MulticastDelegate<int> OnScoreGained;
private:

	void OnNotified(dae::Event e) override;

	dae::User UserID;
	std::string Name;
	std::string Sprite;

	int emeraldStreak{ 0 };

	bool fireBall{ true };
	bool Dead{ false };
	int Lives{ 3 };
	int Score{ 0 };
};

class LifeDisplay final : public dae::Component<LifeDisplay>
{
public:

	void Init(PlayerComponent* user);

private:
	dae::ComponentRef<dae::TextComponent> Text;
	int Lives{ 3 };
};

class ScoreDisplay final : public dae::Component<ScoreDisplay>
{

private:
	dae::ComponentRef<dae::TextComponent> Text;
public:

	void Init();
};

class GoldBag final : public dae::Component<GoldBag>
{
	class State_Falling : public dae::GameState
	{
	public:
		void Init() override;
		void Exit() override;

		State_Falling(GoldBag* b) : dae::GameState("bagfall"), bag(b) {}

		bool fallStopped{false};
		dae::ComponentRef<GoldBag> bag;
		int fallDist{0};
		dae::MulticastDelegate<Grid*, Direction, int, int>::DelegateHandle fallHandle;
	};
	class State_Still : public dae::GameState
	{
	public:
		void Init() override;
		void Exit() override;

		State_Still(GoldBag* b) : dae::GameState("bagstill"), bag(b) {}

		dae::ComponentRef<GoldBag> bag;
	};
	class State_FallingStart : public dae::GameState
	{
	public:
		void Init() override;
		void Update(float) override;
		void Exit() override;

		State_FallingStart(GoldBag* b) : dae::GameState("bagfallstart"), bag(b) {}

		dae::ComponentRef<GoldBag> bag;
		float time{ 1.f };
	};
	class State_Moving : public dae::GameState
	{
	public:
		void Init() override;
		void Exit() override;

		State_Moving(GoldBag* b) : dae::GameState("bagmove"), bag(b) {}

		dae::ComponentRef<GoldBag> bag;
	};
public:

	void ComponentUpdate(float delta) override;

	void OnCreated() override;
	void OnDestroyFinalize() override;

	bool Push(GridMoveComponent*, Direction, dae::GameObject* player);
	bool CanPush() const { return canPush; }
	void SetCanPush(bool state) { canPush = state; }

	dae::GameObject* GetLastPlayer() const { return pushPlayer; }

private:
	dae::ComponentRef<GridMoveComponent> gridmove;
	dae::GameObject* pushPlayer;
	dae::StateManager* bagState;
	Direction pushDir;
	bool canPush;
};

class Enemy final : public dae::Component<Enemy>
{

public:

	void OnCreated() override;
	void OnDestroyed() override;
	void Tick(float delta) override;
	void Init();

	void Possess(dae::User user);

	std::mutex* PathLock;
	std::list<int> Path;
	dae::ComponentRef<GridMoveComponent> MoveComp;
	dae::ComponentRef<PlayerComponent> Player;
private:
	void OnNotified(dae::Event e) override;
	void Boost();

	dae::Timer PathChecker;
	dae::Timer Booster;
	dae::Timer BoostEnd;

	std::string Sprite;
	std::string BoostSprite;
	std::string PlayerSprite;
	std::string PlayerBoostSprite;
};

class PathClearer final : public dae::Component<PathClearer>
{
	
public:

	void OnCreated() override;
	void ComponentUpdate(float delta) override;
	void ClearPath(const std::vector<Direction>& path, int count);
	auto& GetPath() { return Path; }

private:
	int Count;
	size_t Index;
	std::vector<Direction> Path;
	dae::ComponentRef<GridMoveComponent> Mover;
};

class EnemySpawner final : public dae::Component<EnemySpawner>
{

public:

	void StartSpawn(int x, int y, int count = 3);
	void OnDestroyed() override;

private:

	int Count;
	dae::Timer Spawner;
};
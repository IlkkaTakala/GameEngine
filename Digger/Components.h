#pragma once
#include "Data.h"
#include "Delegates.h"
#include "Grid.h"
#include "EngineTime.h"
#include <mutex>

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

	void Init(dae::User user) {
		UserID = user;
	}

	void TakeDamage();
	bool IsAlive() { return Lives > 0; }
	void GiveScore(int amount);

	int GetLives() const { return Lives; }
	int GetScore() const { return Score; }
	dae::User GetID() const { return UserID; }

	dae::MulticastDelegate<> OnDeath;
	dae::MulticastDelegate<> OnDamaged;
	dae::MulticastDelegate<int> OnScoreGained;
private:

	void OnNotified(dae::Event e) override;

	dae::User UserID;

	int emeraldStreak{ 0 };

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

	void Init(PlayerComponent* user);
};

class GoldBag final : public dae::Component<GoldBag>
{

public:
	void StartMoving() {
		IsMoving = true;
	}

	void StopMoving();

	void GridUpdated() {
		fallDist++;
	}

	bool CanCrush() {
		return IsMoving;
	}

	bool CanMove() {
		return IsMoving;
	}

private:

	bool IsMoving{ false };
	int fallDist{ 0 };
};

class Enemy final : public dae::Component<Enemy>
{

public:

	void OnCreated() override;
	void OnDestroyed() override;
	void Tick(float delta) override;
	void Init();

	std::mutex* PathLock;
	std::list<int> Path;
	dae::ComponentRef<GridMoveComponent> MoveComp;
	dae::ComponentRef<PlayerComponent> Player;
private:


	dae::Timer PathChecker;
};

class PathClearer final : public dae::Component<PathClearer>
{
	
public:

	void OnCreated() override;
	void ComponentUpdate(float delta) override;
	void ClearPath(const std::vector<Direction>& path);

private:

	int Index;
	std::vector<Direction> Path;
	dae::ComponentRef<GridMoveComponent> Mover;
};

class EnemySpawner final : public dae::Component<EnemySpawner>
{

public:

	void StartSpawn(int x, int y);
	void OnDestroyed() override;

private:

	dae::Timer Spawner;
};
#include "EngineTime.h"

#include "GameObject.h"
#include "BaseComponent.h"
#include "TransformComponent.h"
#include "InputComponent.h"
#include "SpriteComponent.h"
#include "TextComponent.h"
#include "Texture2D.h"

#include "SystemManager.h"
#include "Components.h"
#include "Factory.h"
#include "AstarSolver.h"

#include <format>
#include <list>
#include "glm/glm.hpp"

using namespace dae;

void GridMoveComponent::OnCreated()
{
	Transform = GetOwner()->GetComponent<dae::TransformComponent>();
	actual = Transform->GetPosition();
	GetCanMove = [](...) { return true; };
	direction = Direction::None;
}

void GridMoveComponent::ComponentUpdate(float /*delta*/)
{
	Transform->SetLocalPosition(glm::vec3(actual, 0.0));
}

void GridMoveComponent::Init(float s, Grid* gridC, int x, int y)
{
	SetSpeed(s);
	SetDirection(Direction::None);
	gridComponent = gridC;
	maxProgress = (float)gridC->Data.size;
	actual = gridC->GetCellCenter(x, y);
	grid = glm::ivec2{ x, y };
	gridC->MoveInto(GetOwner(), x, y);
}

void GridMoveComponent::SetCell(int x, int y)
{
	auto gridC = gridComponent.Get();
	gridC->MoveOut(GetOwner(), grid.x, grid.y);
	actual = gridC->GetCellCenter(x, y);
	grid = glm::ivec2{ x, y };
	progress = 0.f;
	gridC->MoveInto(GetOwner(), x, y);
}

void GridMoveComponent::SetDirection(Direction dir, bool force)
{
	if (!force && IsInProgress()) return;
	direction = dir;
	OnDirectionChanged.Broadcast(dir);
	target = nullptr;
	targetGrid = nullptr;
}

bool GridMoveComponent::Move(float delta)
{
	UpdateDirection();
	if (!target) return true;
	if (GetCanMove) {
		if (!GetCanMove(gridComponent.Get(), direction, grid.x, grid.y)) return true;
	}
	progress += delta * speed;
	int mult = direction == Direction::Up || direction == Direction::Left ? -1 : 1;
	*target += mult * delta * speed;
	OnMoved.Broadcast(gridComponent.Get(), direction, (int)actual.x, (int)actual.y);
	if (progress >= maxProgress) {
		gridComponent->MoveOut(GetOwner(), grid.x, grid.y);
		*targetGrid += mult < 0 ? -1 : 1;
		progress = 0.f;
		actual = gridComponent->GetCellCenter(grid.x, grid.y);
		gridComponent->MoveInto(GetOwner(), grid.x, grid.y);
		OnGridChanged.Broadcast(gridComponent.Get(), direction, grid.x, grid.y);
		direction = Direction::None;
		return true;
	}
	return false;
}

void GridMoveComponent::OnDestroyed()
{
	gridComponent->MoveOut(GetOwner(), grid.x, grid.y);
}

void GridMoveComponent::UpdateDirection()
{
	switch (direction)
	{
	case Direction::Down: {
		if (grid.y < gridComponent->Data.cells_y - 1) {
			target = &actual.y;
			targetGrid = &grid.y;
		}
	} break;
	case Direction::Right: {
		if (grid.x < gridComponent->Data.cells_x - 1) {
			target = &actual.x;
			targetGrid = &grid.x;
		}
	} break;
	case Direction::Up: {
		if (grid.y > 0) {
			target = &actual.y;
			targetGrid = &grid.y;
		}
	} break;
	case Direction::Left: {
		if (grid.x > 0) {
			target = &actual.x;
			targetGrid = &grid.x;
		}
	} break;

	default:
		target = nullptr;
		targetGrid = nullptr;
		break;
	}
}

void PlayerComponent::TakeDamage()
{
	if (Dead) return;
	Lives--;
	SystemManager::GetSoundSystem()->Play("death.wav");
	OnDamaged.Broadcast();
	emeraldStreak = 0;
	Dead = true;
	GetOwner()->GetComponent<GridMoveComponent>()->SetDirection(Direction::None, true);

	if (auto it = GetOwner()->GetComponent<dae::InputComponent>(); it != nullptr) {
		it->SetInputEnabled(false);
	}
	GetOwner()->GetComponent<dae::SpriteComponent>()->SetTexture("VGRAVE5.gif");
	dae::Time::GetInstance().SetTimerByEvent(2.f, [this]() {
		if (auto it = GetOwner()->GetComponent<dae::InputComponent>(); it != nullptr) {
			it->SetInputEnabled(true);
		}
	GetOwner()->GetComponent<GridMoveComponent>()->SetCell(1, 1);
	GetOwner()->GetComponent<dae::SpriteComponent>()->SetTexture("digger.tga");
	Dead = false;
		});

	if (Lives <= 0) {
	}
}

void PlayerComponent::GiveScore(int amount)
{
	if (Dead) return;
	Score += amount;
	OnScoreGained.Broadcast(Score);
	EventHandler::FireEvent(Events::PlayerScoreGained, GetOwner());
}

void PlayerComponent::OnNotified(Event e)
{
	if (Dead) return;
	switch (e.type)
	{
	case Events::GoldBagCrush:
		TakeDamage();
		break;

	case Events::ScoreEmerald:
		e.object->Destroy();
		SystemManager::GetSoundSystem()->Play("collect.wav", { 2.f });
		GiveScore(25);
		if (++emeraldStreak >= 8) {
			emeraldStreak = 0;
			GiveScore(250);
		}
		break;

	case Events::ScoreGold:
		e.object->Destroy();
		SystemManager::GetSoundSystem()->Play("collect.wav", { 2.f });
		GiveScore(500);
		break;

	default:
		break;
	}
}

void LifeDisplay::Init(PlayerComponent* user)
{
	Text = GetOwner()->GetComponent<TextComponent>();
	Text->SetText(std::format("Lives: {}", Lives));
	user->OnDamaged.Bind(GetOwner(), [Disp = GetPermanentReference()]() {
		Disp->Lives--;
	Disp->Text->SetText(std::format("Lives: {}", Disp->Lives));
		});
}

void ScoreDisplay::Init(PlayerComponent* user)
{
	Text = GetOwner()->GetComponent<TextComponent>();
	Text->SetText(std::format("{:05}", 0));

	user->OnScoreGained.Bind(GetOwner(), [Text = Text](int score) {
		Text->SetText(std::format("{:05}", score));
		});
}

void GoldBag::StopMoving()
{
	IsMoving = false;
	if (fallDist >= 2) {
		GetOwner()->Destroy();
		auto g = GetOwner()->GetComponent<GridMoveComponent>()->GetGridLoc();
		makeGold(g.x, g.y);
		SystemManager::GetSoundSystem()->Play("bag_break.wav");
	}
	fallDist = 0;
}

void Enemy::OnCreated()
{
	
}

void Enemy::OnDestroyed()
{
	Time::GetInstance().ClearTimer(PathChecker);
	delete PathLock;
	PathLock = nullptr;
}

void Enemy::Tick(float delta)
{
	if (MoveComp->Move(delta)) {
		if (Path.empty()) return;
		std::scoped_lock lock(*PathLock);
		MoveComp->SetDirection((Direction)Path.front());
		Path.pop_front();
	}
}

typedef std::array<int, 2> Vec2;

inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs) {
	return { lhs[0] - rhs[0], lhs[1] - rhs[1] };
}
inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs) {
	return { lhs[0] + rhs[0], lhs[1] + rhs[1] };
}

void RefreshPath(ComponentRef<Enemy> ref)
{
	static std::vector<Direction> actions = {
		Direction::Up,
		Direction::Right,
		Direction::Down,
		Direction::Left,
	};


	static std::vector<Vec2> dirs = {
		{0, -1},
		{1, 0},
		{0, 1},
		{-1, 0},
	};
	{
		std::scoped_lock lock(*ref->PathLock);
		ref->Path.clear();
	}
	glm::ivec2 start;
	if (ref->MoveComp->IsInProgress()) {
		start = ref->MoveComp->GetGridLoc();
		Vec2& c = dirs[(int)ref->MoveComp->GetDirection()];
		start.x += c[0];
		start.y += c[1];
	}
	else start = ref->MoveComp->GetGridLoc();
	glm::ivec2 targetGlm = ref->Player->GetOwner()->GetComponent<GridMoveComponent>()->GetGridLoc();
	auto gridreal = ref->MoveComp->GetGrid();

	auto validPreCheck = [grid = gridreal->GetPermanentReference()](const Vec2& state, const Direction& action) {
		Vec2 newState = state + dirs[(int)action];
		auto c = grid->GetCell(state[0], state[1]);
		c->Tunnels[(int)action];
		return c && c->Tunnels[(int)action] && c->Tunnels[(int)action]->Cleared;
	};

	auto validPostCheck = [](const Vec2& /*state*/) {
		return true;
	};

	Vec2 target = { targetGlm.x, targetGlm.y };
	auto Visit = [&target](const Vec2& state) {
		return state != target;
	};

	auto Act = [](const Vec2& state, const Direction& action) {
		return state + dirs[(int)action];
	};

	auto StateCost = [](const Vec2& /*state*/, int /*action*/) {
		return 1.f;
	};

	auto PathCost = [](const Vec2& /*state*/, float cost) -> float {
		return (float)cost;
	};
	auto path = Astar::FindActionPath<Vec2, Direction>(
		{ start.x, start.y }, actions, validPreCheck,
		validPostCheck, Visit,
		Act, StateCost, PathCost);

	std::scoped_lock lock(*ref->PathLock);
	ref->Path = path;
}

void Enemy::Init()
{
	auto& obs = PlayerComponent::ObjectList();

	Player = &obs[rand() % obs.size()];
	MoveComp = GetOwner()->GetComponent<GridMoveComponent>();

	PathLock = new std::mutex();

	PathChecker = Time::GetInstance().SetTimerByEvent(1.f, [ref = GetPermanentReference()]() {
		RefreshPath(ref);
	}, true);
}

void PathClearer::OnCreated()
{
	Mover = GetOwner()->GetComponent<GridMoveComponent>();
}

void PathClearer::ComponentUpdate(float delta)
{
	if (Path.empty()) return;

	if (Mover->Move(delta)) {
		
		auto loc = Mover->GetGridLoc();
		if (Index > 2 && Index < Path.size() && Path[Index - 1] != Path[Index])
			Grid::GetObject(0)->EatCell(loc.x, loc.y);
		
		if (Index >= Path.size()) {
			Grid::GetObject(0)->EatCell(loc.x, loc.y);
			GetOwner()->Destroy();
			makeSpawner(loc.x, loc.y);
		}
		else Mover->SetDirection(Path[Index++]);
		
	}
}

void PathClearer::ClearPath(const std::vector<Direction>& path)
{
	Path = path;
	Index = 0;
}

void EnemySpawner::StartSpawn(int x, int y)
{
	Spawner = Time::GetInstance().SetTimerByEvent(2.f, [dx = x, dy = y]() {
		makeEnemy(dx, dy);
	}, true);
}

void EnemySpawner::OnDestroyed()
{
	Time::GetInstance().ClearTimer(Spawner);
}

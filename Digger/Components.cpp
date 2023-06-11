#include "EngineTime.h"

#include "GameObject.h"
#include "BaseComponent.h"
#include "TransformComponent.h"
#include "InputComponent.h"
#include "SpriteComponent.h"
#include "Collider.h"
#include "TextComponent.h"
#include "Texture2D.h"
#include "GameState.h"

#include "SystemManager.h"
#include "Components.h"
#include "Factory.h"
#include "AstarSolver.h"
#include "GameState.h"
#include "GameGlobals.h"

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
	gridComponent = gridC;
	if (!gridC) return;
	SetSpeed(s);
	SetDirection(Direction::None);
	maxProgress = (float)gridC->Data.size;
	SetCell(x, y);
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
	if (!gridComponent.IsValid()) return false;
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

bool PlayerComponent::TakeDamage(bool enemy)
{
	if (Dead) return false;
	if (boosted) {
		if (enemy) GetOwner()->Notify(Events::ScoreEnemy, nullptr);
		return true;
	}
	Lives--;
	SystemManager::GetSoundSystem()->Play("death.wav");
	OnDamaged.Broadcast();
	if (Lives < 0) EventHandler::FireEvent(Events::PlayerFinalScore, GetOwner());
	emeraldStreak = 0;
	Dead = true;
	GetOwner()->GetComponent<GridMoveComponent>()->SetDirection(Direction::None, true);

	if (auto it = GetOwner()->GetComponent<dae::InputComponent>(); it != nullptr) {
		it->SetInputEnabled(false);
	}
	GetOwner()->GetComponent<dae::SpriteComponent>()->SetTexture("VGRAVE5.gif");
	if (Lives >= 0)
		dae::Time::GetInstance().SetTimerByEvent(2.f, [this]() {
			if (auto it = GetOwner()->GetComponent<dae::InputComponent>(); it != nullptr) {
				it->SetInputEnabled(true);
			}
			GetOwner()->GetComponent<GridMoveComponent>()->SetCell(1, 1);
			GetOwner()->GetComponent<dae::SpriteComponent>()->SetTexture(Sprite);
			dae::Time::GetInstance().SetTimerByEvent(0.1f, [this]() {
				Dead = false;
				});
			EventHandler::FireEvent(Events::PlayerDeath, GetOwner());
		});
	else {
		dae::Time::GetInstance().SetTimerByEvent(2.f, [this]() {
			EventHandler::FireEvent(Events::PlayerDeath, GetOwner());
		});
	}
	return true;
}

void PlayerComponent::GiveScore(int amount)
{
	if (Dead) return;
	Score += amount;
	GameGlobals::GetInstance().AddScore(amount);
	OnScoreGained.Broadcast(Score);
	EventHandler::FireEvent(Events::PlayerScoreGained, GetOwner());
}

void PlayerComponent::Boost()
{
	boosted = true;

	auto obj = makeFlash();

	Time::GetInstance().SetTimerByEvent(4.f, [obj, this] {
		obj->Destroy();
		boosted = false;
		});
}

bool PlayerComponent::TryFireball()
{
	if (fireBall && !Dead) {
		fireBall = false;
		GetOwner()->GetComponent<dae::SpriteComponent>()->SetTexture("fire_" + Sprite);
		Time::GetInstance().SetTimerByEvent(5.f, [this] {
			GetOwner()->GetComponent<dae::SpriteComponent>()->SetTexture(Sprite);
			fireBall = true;
		});

		return true;
	}
	return false;
}

void PlayerComponent::LevelChanged(int x, int y)
{
	if (Lives < 0) {
		GetOwner()->SetActive(true);
		return;
	};
	GetOwner()->GetComponent<GridMoveComponent>()->Init(150.f, Grid::GetObject(0), x, y);
	Grid::GetObject(0)->EatCell(x, y);
	GetOwner()->GetComponent<dae::SpriteComponent>()->SetTexture(Sprite);
	GetOwner()->GetComponent<dae::TransformComponent>()->SetLocalRotation(0.f);

	Time::GetInstance().SetTimerByEvent(0.3f, [this] {
		Dead = false;
		InputManager::GetInstance().SetUserMapping(UserID, "Default");
		GetOwner()->SetActive(true);
	});
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
		EventHandler::FireEvent(Events::EmeraldDestroy, e.object);
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

	case Events::ScoreEnemy:
		GiveScore(250);
		break;
	case Events::BonusPickup:
		e.object->Destroy();
		Boost();
		break;

	default:
		break;
	}
}

void LifeDisplay::Init(PlayerComponent* user)
{
	Lives = user->GetLives();
	Text = GetOwner()->GetComponent<TextComponent>();
	Text->SetText(std::format("Lives: {}", Lives));
	user->OnDamaged.Bind(GetOwner(), [Disp = GetPermanentReference()]() {
		Disp->Lives--;
		Disp->Text->SetText(std::format("Lives: {}", Disp->Lives > 0 ? Disp->Lives : 0));
	});
}

void ScoreDisplay::Init()
{
	Text = GetOwner()->GetComponent<TextComponent>();
	Text->SetText(std::format("{:05}", 0));

	GameGlobals::GetInstance().ScoreGained.Bind(GetOwner(), [Text = Text](int score) {
		Text->SetText(std::format("{:05}", score));
		});
}

void GoldBag::ComponentUpdate(float delta)
{
	if (bagState) bagState->CheckState();

	auto m = gridmove.Get();
	if (m) {
		auto dir = m->GetDirection();
		if (m->Move(delta)) {
			pushDir = Direction::None;
			m->SetDirection(dir);
		}
	}
	if (bagState) bagState->UpdateState(delta);
}

void GoldBag::OnCreated()
{
	pushDir = Direction::None;
	bagState = new StateManager();
	auto state1 = new State_Still(this);
	auto state2 = new State_Moving(this);
	auto state3 = new State_Falling(this);
	auto state4 = new State_FallingStart(this);
	auto still = bagState->AddState(state1);
	auto move  = bagState->AddState(state2);
	auto fall  = bagState->AddState(state3);
	auto sfall = bagState->AddState(state4);

	gridmove = GetOwner()->GetComponent<GridMoveComponent>();
	auto grid = gridmove;
	bagState->AddPath(still, move, [ref = GetPermanentReference()](GameState*) -> bool {
		return ref->pushDir != Direction::None;
	});
	bagState->AddPath(still, sfall, [grid](GameState*) -> bool {
		auto Cell = grid->GetGrid()->GetCellInDirection(Direction::Down, grid->GetGridLoc().x, grid->GetGridLoc().y);
		auto Cell2 = grid->GetGrid()->GetCell(grid->GetGridLoc().x, grid->GetGridLoc().y);
		return Cell->Cleared && !Cell2->Cleared && Cell->Objects.empty();
	});
	bagState->AddPath(still, fall, [grid](GameState*) -> bool {
		auto Cell = grid->GetGrid()->GetCellInDirection(Direction::Down, grid->GetGridLoc().x, grid->GetGridLoc().y);
		auto Cell2 = grid->GetGrid()->GetCell(grid->GetGridLoc().x, grid->GetGridLoc().y);
		return Cell->Cleared && Cell2->Cleared && Cell->Objects.size() == 0;
	});
	bagState->AddPath(move, fall, [state2, grid](GameState*) -> bool {
		auto Cell = grid->GetGrid()->GetCellInDirection(Direction::Down, grid->GetGridLoc().x, grid->GetGridLoc().y);
		return Cell->Cleared;
	});
	bagState->AddPath(move, still, [ref = GetPermanentReference(), grid](GameState*) -> bool {
		return ref->pushDir == Direction::None;
	});
	bagState->AddPath(fall, still, [state3](GameState*) -> bool {
		return state3->fallStopped;
	});
	bagState->AddPath(sfall, fall, [grid, state4](GameState*) -> bool {
		return state4->time <= 0.f;
	});

}

void GoldBag::OnDestroyFinalize()
{
	delete bagState;
	bagState = nullptr;
}

bool GoldBag::Push(GridMoveComponent* g, Direction dir, GameObject* player)
{
	if (!canPush) return false;
	if (gridmove->GetDirection() == Direction::None || gridmove->GetDirection() == dir) {
		if (gridmove->GetGridLoc() - g->GetGridLoc() == Opposites[(int)dir]) {
			pushDir = dir;
			pushPlayer = player;
			return true;
		}
	} 
	return false;
}

void Enemy::OnCreated()
{
	
}

void Enemy::OnDestroyed()
{
	EventHandler::FireEvent(Events::EnemyDestroy, GetOwner());
	Time::GetInstance().ClearTimer(PathChecker);
	Time::GetInstance().ClearTimer(Booster);
	Time::GetInstance().ClearTimer(BoostEnd);
	delete PathLock;
	PathLock = nullptr;
}

void Enemy::Tick(float delta)
{
	if (GetOwner()->HasComponent<InputComponent>()) return;
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
	if (!ref->IsValid()) return;
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

	auto validPreCheck = [grid = gridreal->GetPermanentReference(), ref = ref->MoveComp](const Vec2& state, const Direction& action) {
		Vec2 newState = state + dirs[(int)action];
		return ref->GetCanMove(grid.Get(), action, state[0], state[1]);
		/*auto c = grid->GetCell(state[0], state[1]);
		c->Tunnels[(int)action];
		return c && c->Tunnels[(int)action] && c->Tunnels[(int)action]->Cleared;*/
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
	MoveComp->GetCanMove = [](Grid* g, Direction dir, int x, int y) {
		auto c = g->GetCell(x, y);
		c->Tunnels[(int)dir];
		return c && c->Tunnels[(int)dir] && c->Tunnels[(int)dir]->Cleared;
	};

	Sprite = "VRHOB2.png";
	BoostSprite = "VNOB2.png";
	PlayerSprite = "VPHOB.png";
	PlayerBoostSprite = "VPNOB.png";

	GetOwner()->GetComponent<SpriteComponent>()->SetTexture(Sprite);
	
	PathLock = new std::mutex();

	PathChecker = Time::GetInstance().SetTimerByEvent(1.f, [ref = GetPermanentReference()]() {
		RefreshPath(ref);
	}, true);

	Booster = Time::GetInstance().SetTimerByEvent(10.f, [ref = GetPermanentReference()] {
		ref->Boost();
	}, true);
}

void Enemy::Possess(User user)
{
	GetOwner()->GetComponent<SpriteComponent>()->SetTexture(PlayerSprite);
	auto input = CreateComponent<dae::InputComponent>(GetOwner());
	MoveComp->SetSpeed(100.f);
	input->SetUserFocus(user);
	input->Bind2DAction("Move", [ref = MoveComp](float x, float y) {
		auto grid = ref.Get<GridMoveComponent>();
		float delta = sqrt(y * y + x * x) * dae::Time::GetInstance().GetDelta();
		if (grid->Move(delta)) {
			bool up = abs(x) < abs(y);
			if (up && y > 0.f) grid->SetDirection(Direction::Up);
			else if (up && y < 0.f) grid->SetDirection(Direction::Down);
			else if (!up && x > 0.f) grid->SetDirection(Direction::Right);
			else if (!up && x < 0.f) grid->SetDirection(Direction::Left);
		}
	});
}

void Enemy::OnNotified(Event e)
{
	if (e.type == Events::GoldBagCrush) {
		e.object->Notify(Events::ScoreEnemy, GetOwner());
		GetOwner()->Destroy();
	}
}

void Enemy::Boost()
{
	MoveComp->GetCanMove = [](Grid*, Direction, int, int) {
		return true;
	};
	if (GetOwner()->HasComponent<InputComponent>())
		GetOwner()->GetComponent<SpriteComponent>()->SetTexture(PlayerBoostSprite);
	else GetOwner()->GetComponent<SpriteComponent>()->SetTexture(BoostSprite);


	BoostEnd = Time::GetInstance().SetTimerByEvent(4.f, [ref = MoveComp, ref2 = GetPermanentReference()] {
		ref->GetCanMove = [](Grid* g, Direction dir, int x, int y) {
			auto c = g->GetCell(x, y);
			c->Tunnels[(int)dir];
			return c && c->Tunnels[(int)dir] && c->Tunnels[(int)dir]->Cleared;
		};
		if (ref2->GetOwner()->HasComponent<InputComponent>())
			ref2->GetOwner()->GetComponent<SpriteComponent>()->SetTexture(ref2->PlayerSprite);
		else 
			ref2->GetOwner()->GetComponent<SpriteComponent>()->SetTexture(ref2->Sprite);
	});
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
			makeSpawner(loc.x, loc.y, Count);
		}
		else Mover->SetDirection(Path[Index++]);
		
	}
}

void PathClearer::ClearPath(const std::vector<Direction>& path, int count)
{
	Count = count;
	Path = path;
	Index = 0;
}

void EnemySpawner::StartSpawn(int x, int y, int count)
{
	Count = count;
	Spawner = Time::GetInstance().SetTimerByEvent(2.f, [dx = x, dy = y, ref = GetPermanentReference()]() {
		auto en = makeEnemy(dx, dy);
		EventHandler::FireEvent(Events::EnemySpawn, en);
		if (--ref->Count <= 0) {
			Time::GetInstance().ClearTimer(ref->Spawner);
			ref->GetOwner()->Destroy();
		}
	}, true);
}

void EnemySpawner::OnDestroyed()
{
	Time::GetInstance().ClearTimer(Spawner);
}

void GoldBag::State_Falling::Init()
{
	fallStopped = false;
	auto grid = bag->gridmove;
	grid->SetDirection(Direction::Down, true);

	fallHandle = grid->OnGridChanged.Bind(bag->GetOwner(), [this](Grid*, Direction, int, int) {
		fallDist++;
	});

	auto overlap = bag->GetOwner()->GetComponent<SphereOverlap>();
	overlap->OnCollision.Bind(bag->GetOwner(), [p = bag->pushPlayer](GameObject* self, GameObject* other) {
		auto o = self->GetComponent<GridMoveComponent>();
		auto cell = Grid::GetObject(0)->GetCellInDirection(Direction::Down, o->GetGridLoc().x, o->GetGridLoc().y);
		auto cell2 = Grid::GetObject(0)->GetCell(o->GetGridLoc().x, o->GetGridLoc().y);
		if (cell) {
			bool found = false;
			for (auto& ob : cell->Objects) {
				if (ob == other) {
					found = true;
					break;
				}
			}
			if (!found)
				for (auto& ob : cell2->Objects) {
					if (ob == other) {
						found = true;
						break;
					}
				}
			if (found) other->Notify(Events::GoldBagCrush, p);
		}
	});

	bag->gridmove->GetCanMove = [this, ref = bag->GetPermanentReference()](Grid* g, Direction dir, int x, int y) -> bool {
		auto Cell = g->GetCellInDirection(dir, x, y);
		if (Cell)
			if (Cell->Cleared) {
				return true;
			}
		fallStopped = true;
		return false;
	};
}

void GoldBag::State_Falling::Exit()
{
	auto grid = bag->gridmove;
	auto overlap = bag->GetOwner()->GetComponent<SphereOverlap>();
	grid->OnGridChanged.Unbind(fallHandle);
	overlap->OnCollision.UnbindAll();
	if (fallDist >= 2) {
		bag->GetOwner()->Destroy();
		auto g = bag->gridmove->GetGridLoc();
		makeGold(g.x, g.y);
		SystemManager::GetSoundSystem()->Play("bag_break.wav");
	}
	fallDist = 0;
}

void GoldBag::State_Still::Init()
{
	bag->SetCanPush(true);
	auto grid = bag->gridmove;
	grid->SetDirection(Direction::None, true);
	grid->GetCanMove = [ref = bag->GetPermanentReference()](Grid* g, Direction dir, int x, int y) -> bool {
		auto Cell = g->GetCellInDirection(dir, x, y);
		if (Cell)
			if (!Cell->Objects.empty()) {
				if (auto m = Cell->Objects[0]->GetComponent<GridMoveComponent>(); m != nullptr) {
					return m->GetCanMove(g, dir, m->GetGridLoc().x, m->GetGridLoc().y);
				}
			}
			else return true;
		return false;
	};
}

void GoldBag::State_Still::Exit()
{
	bag->SetCanPush(false);
}

void GoldBag::State_FallingStart::Init()
{
	time = 1.f;
}

void GoldBag::State_FallingStart::Update(float delta)
{
	time -= delta;
	auto trans = bag->GetOwner()->GetComponent<TransformComponent>();
	trans->SetLocalPosition(trans->GetLocalPosition() + glm::vec3{ sin(time * 5.f) * 3.f, 0, 0 });
}

void GoldBag::State_FallingStart::Exit()
{
}

void GoldBag::State_Moving::Init()
{
	auto overlap = bag->GetOwner()->GetComponent<SphereOverlap>();
	overlap->OnCollision.Bind(bag->GetOwner(), [b = bag](GameObject* self, GameObject* other) {
		auto o = self->GetComponent<GridMoveComponent>();
		if (other->HasComponent<GoldBag>()) {
			auto g = other->GetComponent<GoldBag>();
			g->Push(o, o->GetDirection(), b->GetLastPlayer());
			
		}
	});
	bag->gridmove->SetDirection(bag->pushDir);
	bag->gridmove->GetCanMove = [ref = bag->GetPermanentReference()](Grid* g, Direction dir, int x, int y) -> bool {
		auto Cell = g->GetCellInDirection(dir, x, y);
		if (Cell)
			if (!Cell->Objects.empty()) {
				if (auto m = Cell->Objects[0]->GetComponent<GridMoveComponent>(); m != nullptr) {
					return m->GetCanMove(g, m->GetDirection() == Direction::None ? dir : m->GetDirection(), m->GetGridLoc().x, m->GetGridLoc().y);
				}
			}
			else return true;
		return false;
	};
}

void GoldBag::State_Moving::Exit()
{
	auto overlap = bag->GetOwner()->GetComponent<SphereOverlap>();
	overlap->OnCollision.UnbindAll();
}

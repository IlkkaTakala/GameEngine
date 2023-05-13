#include "Factory.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "InputManager.h"
#include "InputComponent.h"
#include "Components.h"
#include "ResourceManager.h"
#include "Collider.h"
#include "EngineTime.h"
#include "SystemManager.h"

void makeDisplay(PlayerComponent* player, dae::Scene& scene)
{
	using namespace dae;

	auto root = new GameObject();
	auto trans = CreateComponent<TransformComponent>(root);
	trans->SetPosition({ 10, 10 + 85 * player->GetID(), 0 });

	auto go1 = new GameObject();
	trans = CreateComponent<TransformComponent>(go1);
	auto text = CreateComponent<TextComponent>(go1);
	auto life = CreateComponent<LifeDisplay>(go1);
	text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
	life->Init(player);
	trans->SetLocalPosition({ 100, 0, 0 });
	go1->SetParent(root);

	auto go2 = new GameObject();
	trans = CreateComponent<TransformComponent>(go2);
	text = CreateComponent<TextComponent>(go2);
	auto score = CreateComponent<ScoreDisplay>(go2);
	text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
	text->SetColor(SDL_Color{ 0, 200, 0, 255 });
	score->Init(player);
	trans->SetLocalPosition({ 10, 0, 0 });
	go2->SetParent(root);

	scene.Add(root);
}

void makePlayer(dae::User user, dae::Scene& scene, float speed)
{
	using namespace dae;

	auto go1 = new GameObject();
	auto player = CreateComponent<PlayerComponent>(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);
	auto input = CreateComponent<InputComponent>(go1);
	player->Init(user);

	overlap->SetRadius(TileSize / 2);

	sprite->SetTexture(user == 0 ? "digger.tga" : "digger2.tga");
	sprite->SetSize(glm::vec3{ SpriteSize });
	input->SetUserFocus(user);
	InputManager::GetInstance().SetUserMapping(user, "Default");
	auto gridmove = CreateComponent<GridMoveComponent>(go1);

	int x = 1;
	int y = 1;
	gridmove->Init(speed, Grid::GetObject(0), x, y);
	Grid::GetObject(0)->EatCell(x, y);
	gridmove->OnGridChanged.Bind(go1, [](Grid* grid, Direction dir, int x, int y) {
		grid->ClearCell(dir, x, y);
		});
	gridmove->OnMoved.Bind(go1, [](Grid* grid, Direction dir, int x, int y) {
		grid->Eat(dir, x, y);
		});
	gridmove->GetCanMove = [](Grid* g, Direction dir, int x, int y) -> bool {
		bool Hor = dir == Direction::Left || dir == Direction::Right;
		auto Cell = g->GetCellInDirection(dir, x, y);
		if (Cell) {
			if (Cell->Objects.empty()) return true;
			if (Hor)
				if (auto m = Cell->Objects[0]->GetComponent<GridMoveComponent>(); m != nullptr) {
					return m->GetCanMove(g, dir, m->GetGridLoc().x, m->GetGridLoc().y);
				}
		}
		return false;
	};
	gridmove->OnDirectionChanged.Bind(go1, [ref = trans->GetPermanentReference()](Direction dir) {
		auto t = ref.Get();
		t->SetLocalRotation(0.f);
		t->SetLocalScale({ 1,1,1 });
		switch (dir)
		{
		case Direction::Up:
			t->SetLocalRotation(-90.f);
			break;
		case Direction::Right:
			break;
		case Direction::Down:
			t->SetRotation(90.f);
			break;
		case Direction::Left:
			t->SetScale({ -1, 1, 1 });
			break;
		default:
			break;
		}
		});

	input->Bind2DAction("Move", [ref = go1->GetComponent<GridMoveComponent>()->GetPermanentReference()](float x, float y) {
		auto grid = ref.Get<GridMoveComponent>();
		float delta = sqrt(y * y + x * x) * Time::GetInstance().GetDelta();
		if (grid->Move(delta)) {
			bool up = abs(x) < abs(y);
			if (up && y > 0.f) grid->SetDirection(Direction::Up);
			else if (up && y < 0.f) grid->SetDirection(Direction::Down);
			else if (!up && x > 0.f) grid->SetDirection(Direction::Right);
			else if (!up && x < 0.f) grid->SetDirection(Direction::Left);
		}
	});

	makeDisplay(player, scene);

	scene.Add(go1);
}

void makeGold(int x, int y)
{
	using namespace dae;

	auto go1 = new GameObject();
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);

	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		other->Notify(Events::ScoreGold, self);
		});
	auto g = Grid::GetObject(0);
	trans->SetPosition(g->GetCellCenter(x, y));

	overlap->SetRadius(SpriteSize / 2);
	sprite->SetTexture("VGOLD3.png");
	sprite->SetSize(SpriteSize);
}

void makeEnemy(int x, int y)
{
	using namespace dae;

	auto go1 = new GameObject();
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	/*auto trans = */CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);
	auto enemy = CreateComponent<Enemy>(go1);
	auto grid = CreateComponent<GridMoveComponent>(go1);

	grid->Init(100.f, Grid::GetObject(0), x, y);
	grid->GetCanMove = [](Grid* g, Direction dir, int x, int y) {
		auto c = g->GetCell(x, y);
		c->Tunnels[(int)dir];
		return c && c->Tunnels[(int)dir] && c->Tunnels[(int)dir]->Cleared;
	};
	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		if (other->HasComponent<PlayerComponent>()) {
			other->GetComponent<PlayerComponent>()->TakeDamage();
			SystemManager::GetSoundSystem()->Play("bite.wav");
			self->Destroy();
		}
	});
	enemy->Init();

	overlap->SetRadius(20.f);
	sprite->SetTexture("enemy.tga");
}

void makeEmerald()
{
	using namespace dae;

	auto go1 = new GameObject();
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);

	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		other->Notify(Events::ScoreEmerald, self);
	});
	auto g = Grid::GetObject(0);
	trans->SetPosition(g->GetCellCenter(rand() % (g->Data.cells_x - 1), rand() % (g->Data.cells_y - 1)));

	overlap->SetRadius(SpriteSize / 2);
	sprite->SetTexture("VEMERALD.png");
	sprite->SetSize(SpriteSize * 0.8f);
}

void makeGoldBag()
{
	using namespace dae;
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	auto go = new GameObject();

	/*auto trans =*/ CreateComponent<TransformComponent>(go);
	auto sprite = CreateComponent<SpriteComponent>(go);
	auto overlap = CreateComponent<SphereOverlap>(go);
	auto grid = CreateComponent<GridMoveComponent>(go);
	auto bag = CreateComponent<GoldBag>(go);
	Grid* g = Grid::GetObject(0);
	grid->Init(150, g, rand() % (g->Data.cells_x - 1), rand() % (g->Data.cells_y / 2));

	grid->GetCanMove = [ref = bag->GetPermanentReference()](Grid* g, Direction dir, int x, int y) -> bool {
		bool Hor = dir == Direction::Left || dir == Direction::Right;
		auto Cell = g->GetCellInDirection(dir, x, y);
		if (Cell)
			if (Cell->Cleared && !Hor) {
				if (ref->CanMove()) {
					return true;
				}
				if (Cell->Objects.empty()) {
					ref->StartMoving();
					return true;
				}
			}
			else if (!Cell->Objects.empty() && Hor) {
				if (auto m = Cell->Objects[0]->GetComponent<GridMoveComponent>(); m != nullptr) {
					return m->GetCanMove(g, dir, m->GetGridLoc().x, m->GetGridLoc().y);
				}
			}
			else if (Hor) return true;
		ref->StopMoving();
		return false;
	};
	grid->OnGridChanged.Bind(go, [ref = bag->GetPermanentReference()](Grid*, Direction dir, int, int) {
		if (dir == Direction::Down) ref->GridUpdated();
		});
	grid->OnMoved.Bind(go, [](Grid* grid, Direction dir, int x, int y) {
		grid->Eat(dir, x, y);
		});

	go->AddTickSystem([](GameObject* g, float delta) {
		auto m = g->GetComponent<GridMoveComponent>();
	if (m) {
		Direction ori = m->GetDirection();
		m->SetDirection(Direction::Down);
		m->Move(delta);
		m->SetDirection(ori);
	}
		});

	overlap->SetRadius(20.f);
	overlap->OnCollision.Bind(go, [](GameObject* self, GameObject* other) {
		auto o = self->GetComponent<GridMoveComponent>();
	if (self->GetComponent<GoldBag>()->CanCrush()) {
		auto cell = Grid::GetObject(0)->GetCellInDirection(Direction::Down, o->GetGridLoc().x, o->GetGridLoc().y);
		if (cell) {
			bool found = false;
			for (auto& ob : cell->Objects) {
				if (ob == other) {
					found = true;
					break;
				}
			}
			if (found) other->Notify(Events::GoldBagCrush, self);
		}
	}
	if (other->HasComponent<GridMoveComponent>()) {
		auto g = other->GetComponent<GridMoveComponent>();
		if (g->GetDirection() == Direction::Left || g->GetDirection() == Direction::Right) {
			if (g->GetGridLoc() == (o->GetGridLoc() - Opposites[(int)g->GetDirection()])) {
				o->SetDirection(g->GetDirection());
				o->Move(Time::GetInstance().GetDelta());
			}
		}
	}
		});

	sprite->SetTexture("VSBAG.gif");
	sprite->SetSize(glm::vec3{ SpriteSize });

	Scene->Add(go);
}

void makeClearer()
{
	using namespace dae;
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	auto go = new GameObject();

	/*auto trans =*/ CreateComponent<TransformComponent>(go);
	auto grid = CreateComponent<GridMoveComponent>(go);
	auto clear = CreateComponent<PathClearer>(go);
	Grid* g = Grid::GetObject(0);
	grid->Init(600, g, 1, 1);
	Grid::GetObject(0)->EatCell(1, 1);
	grid->OnGridChanged.Bind(go, [](Grid* grid, Direction dir, int x, int y) {
		grid->ClearCell(dir, x, y);
	});
	grid->OnMoved.Bind(go, [](Grid* grid, Direction dir, int x, int y) {
		grid->Eat(dir, x, y);
	});
	clear->ClearPath({
		Direction::Right,
		Direction::Right,
		Direction::Right,
		Direction::Right,
		Direction::Down,
		Direction::Down,
		Direction::Down,
		Direction::Down,
		Direction::Down,
		Direction::Right,
		Direction::Right,
		Direction::Right,
		Direction::Up,
		Direction::Up,
		Direction::Up,
		Direction::Up,
		Direction::Up,
		Direction::Up,
		Direction::Right,
		Direction::Right,
		Direction::Right,
		Direction::Right,
	});

	Scene->Add(go);
}

void makeSpawner(int x, int y)
{
	using namespace dae;
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	auto go = new GameObject();

	auto spawn = CreateComponent<EnemySpawner>(go);
	spawn->StartSpawn(x, y);

	Scene->Add(go);
}

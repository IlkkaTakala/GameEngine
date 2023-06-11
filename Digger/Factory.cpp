#include "Factory.h"
#include "GameObject.h"
#include <format>

#include "TransformComponent.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "InputManager.h"
#include "InputComponent.h"
#include "UIComponent.h"

#include "Components.h"
#include "Texture2D.h"

#include "ResourceManager.h"
#include "Collider.h"
#include "EngineTime.h"
#include "SystemManager.h"
#include "File.h"
#include "Data.h"
#include "GameGlobals.h"

using namespace dae;

void makeDisplay(PlayerComponent* player, dae::Scene& scene)
{

	auto root = new GameObject("statview");
	auto trans = CreateComponent<TransformComponent>(root);
	trans->SetPosition({ 10, 10 + 85 * player->GetID(), 0 });

	auto go1 = new GameObject("lifeview");
	trans = CreateComponent<TransformComponent>(go1);
	auto text = CreateComponent<TextComponent>(go1);
	auto life = CreateComponent<LifeDisplay>(go1);
	text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
	life->Init(player);
	trans->SetLocalPosition({ 100, 0, 0 });
	go1->SetParent(root);

	auto go2 = new GameObject("scoreview");
	trans = CreateComponent<TransformComponent>(go2);
	text = CreateComponent<TextComponent>(go2);
	auto score = CreateComponent<ScoreDisplay>(go2);
	text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
	text->SetColor(SDL_Color{ 0, 200, 0, 255 });
	score->Init();
	trans->SetLocalPosition({ 10, 0, 0 });
	go2->SetParent(root);

	scene.Add(root);
}

GameObject* makePlayer(dae::User user, dae::Scene& scene, float speed, int x, int y)
{

	auto go1 = new GameObject("player");
	auto player = CreateComponent<PlayerComponent>(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);
	auto input = CreateComponent<InputComponent>(go1);
	const char* spritetxt = user == 0 ? "digger.png" : "digger2.png";

	player->Init(user, GameGlobals::GetInstance().GetPlayerName(user), spritetxt);

	overlap->SetRadius(TileSize / 2);

	sprite->SetTexture(spritetxt);
	sprite->SetSize(glm::vec3{ SpriteSize });
	input->SetUserFocus(user);
	InputManager::GetInstance().SetUserMapping(user, "Default");
	auto gridmove = CreateComponent<GridMoveComponent>(go1);

	gridmove->Init(speed, Grid::GetObject(0), x, y);
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

	overlap->OnCollision.Bind(go1, [go1, ref = gridmove->GetPermanentReference()](GameObject*, GameObject* other) {
		if (auto it = other->GetComponent<GoldBag>()) {
			if (ref->GetDirection() == Direction::Left || ref->GetDirection() == Direction::Right)
				it->Push(ref.Get(), ref->GetDirection(), go1);
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
	input->BindAction("Skip", [ref = go1->GetComponent<GridMoveComponent>()->GetPermanentReference()]() {
		EventHandler::FireEvent(Events::SkipLevel, nullptr);
	});
	input->BindAction("Fire", [ref = go1]() {
		if (ref->GetComponent<PlayerComponent>()->TryFireball())
		makeFireball(ref);
	});
	input->BindAction("Mute", [ref = go1]() {
		GameGlobals::GetInstance().ToggleMuted();
	});

	//makeDisplay(player, scene);

	scene.Add(go1);

	return go1;
}

void makeHUD(GameObject* player1, GameObject* player2)
{
	auto* scene = SceneManager::GetInstance().GetCurrentScene();

	auto root = new GameObject("hud");
	auto trans = CreateComponent<TransformComponent>(root);
	trans->SetPosition({ 10, 10, 0 });

	auto go1 = new GameObject("lifeview1");
	trans = CreateComponent<TransformComponent>(go1);
	auto text = CreateComponent<TextComponent>(go1);
	auto life = CreateComponent<LifeDisplay>(go1);
	text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
	life->Init(player1->GetComponent<PlayerComponent>());
	trans->SetLocalPosition({ 100, 0, 0 });
	go1->SetParent(root);

	if (player2) {
		auto go3 = new GameObject("lifeview2");
		trans = CreateComponent<TransformComponent>(go3);
		text = CreateComponent<TextComponent>(go3);
		life = CreateComponent<LifeDisplay>(go3);
		text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
		life->Init(player2->GetComponent<PlayerComponent>());
		trans->SetLocalPosition({ 200, 0, 0 });
		go3->SetParent(root);
	}

	auto go2 = new GameObject("scoreview");
	trans = CreateComponent<TransformComponent>(go2);
	text = CreateComponent<TextComponent>(go2);
	auto score = CreateComponent<ScoreDisplay>(go2);
	text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
	text->SetColor(SDL_Color{ 0, 200, 0, 255 });
	score->Init();
	trans->SetLocalPosition({ 10, 0, 0 });
	go2->SetParent(root);

	scene->Add(root);
}

void makeGold(int x, int y)
{

	auto go1 = new GameObject("gold");
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

GameObject* makeEnemy(int x, int y)
{

	auto go1 = new GameObject("enemy");
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	/*auto trans = */CreateComponent<TransformComponent>(go1);
	/*auto sprite =*/ CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);
	auto enemy = CreateComponent<Enemy>(go1);
	auto grid = CreateComponent<GridMoveComponent>(go1);

	grid->Init(75.f, Grid::GetObject(0), x, y);
	grid->OnGridChanged.Bind(go1, [](Grid* grid, Direction dir, int x, int y) {
		grid->ClearCell(dir, x, y);
		});
	grid->OnMoved.Bind(go1, [](Grid* grid, Direction dir, int x, int y) {
		grid->Eat(dir, x, y);
		});
	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		if (other->HasComponent<PlayerComponent>()) {
			if (other->GetComponent<PlayerComponent>()->TakeDamage(true)) {
				SystemManager::GetSoundSystem()->Play("bite.wav");
				self->Destroy();
			}
		}
	});
	enemy->Init();

	overlap->SetRadius(20.f);

	return go1;
}

void makeEmerald(int x, int y)
{
	auto go1 = new GameObject("emerald");
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);

	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		other->Notify(Events::ScoreEmerald, self);
	});
	auto g = Grid::GetObject(0);
	trans->SetPosition(g->GetCellCenter(x, y));

	overlap->SetRadius(SpriteSize / 2);
	sprite->SetTexture("VEMERALD.png");
	sprite->SetSize(SpriteSize * 0.8f);
}

void makeGoldBag(int x, int y)
{
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	auto go = new GameObject("goldbag");

	/*auto trans =*/ CreateComponent<TransformComponent>(go);
	auto sprite = CreateComponent<SpriteComponent>(go);
	auto overlap = CreateComponent<SphereOverlap>(go);
	auto grid = CreateComponent<GridMoveComponent>(go);
	Grid* g = Grid::GetObject(0);
	grid->Init(150, g, x, y);
	/*auto bag =*/ CreateComponent<GoldBag>(go);

	grid->OnMoved.Bind(go, [](Grid* grid, Direction dir, int x, int y) {
		grid->Eat(dir, x, y);
	});

	overlap->SetRadius(8.f);
	sprite->SetTexture("VSBAG.gif");
	sprite->SetSize(glm::vec3{ SpriteSize });

	Scene->Add(go);
}

void makeClearer(int x, int y, int count, const std::vector<Direction>& path)
{
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	auto go = new GameObject("clearer");

	/*auto trans =*/ CreateComponent<TransformComponent>(go);
	auto grid = CreateComponent<GridMoveComponent>(go);
	auto clear = CreateComponent<PathClearer>(go);
	Grid* g = Grid::GetObject(0);
	grid->Init(600, g, x, y);
	Grid::GetObject(0)->EatCell(x, y);
	grid->OnGridChanged.Bind(go, [](Grid* grid, Direction dir, int x, int y) {
		grid->ClearCell(dir, x, y);
	});
	grid->OnMoved.Bind(go, [](Grid* grid, Direction dir, int x, int y) {
		grid->Eat(dir, x, y);
	});
	clear->ClearPath(path, count);

	/**/

	Scene->Add(go);
}

void makeSpawner(int x, int y, int count)
{
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	auto go = new GameObject("spawner");

	auto spawn = CreateComponent<EnemySpawner>(go);
	spawn->StartSpawn(x, y, count);

	Scene->Add(go);
}

void makeBonus(int x, int y)
{
	auto go1 = new GameObject("bonus");
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);

	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		other->Notify(Events::BonusPickup, self);
		});
	auto g = Grid::GetObject(0);
	trans->SetPosition(g->GetCellCenter(x, y));

	overlap->SetRadius(SpriteSize / 2);
	sprite->SetTexture("VBONUS.png");
	sprite->SetSize(SpriteSize * 0.8f);
}

void makeFireball(dae::GameObject* player)
{
	auto go1 = new GameObject("fireball");
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);
	auto grid = CreateComponent<GridMoveComponent>(go1);

	auto pgrid = player->GetComponent<GridMoveComponent>();
	auto dir = pgrid->GetDirection();
	auto loc = Opposites[(int)dir] + pgrid->GetGridLoc();

	grid->Init(200.f, Grid::GetObject(0), loc.x, loc.y);
	grid->OnGridChanged.Bind(go1, [go1](Grid* grid, Direction, int x, int y) {
		auto c = grid->GetCell(x, y);
		if (!c || !c->Cleared) {
			go1->Destroy();
		}
		});
	overlap->OnCollision.Bind(go1, [player](GameObject* self, GameObject* other) {
		if (other->IsType("enemy")) {
			other->Destroy();
			self->Destroy();
			player->Notify(Events::ScoreEnemy, self);
		}
		});

	go1->AddTickSystem([dir, ref = grid->GetPermanentReference()](GameObject*, float delta) {
		if (ref->Move(delta)) {
			ref->SetDirection(dir);
		}
	});

	overlap->SetRadius(SpriteSize / 3);
	sprite->SetTexture("VEMERALD.png");
	sprite->SetSize(SpriteSize * 0.2f);
}

GameObject* makeFlash()
{
	auto go1 = new GameObject("bonus");
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);


	auto size = Renderer::GetInstance().GetWindowSize();
	trans->SetPosition({ size / 2, 0.f });
	sprite->SetTexture("bg.png");
	sprite->SetSize(size);

	return go1;
}

LevelData LoadLevel(const std::string& path)
{
	File f = File::OpenFile(path);
	if (!f) return { };

	LevelData Data;
	int cx = 0, cy = 0, type = 0;
	int Margin = 20;
	int Width = 840 - Margin * 2;
	int Height = 480 - Margin * 2;
	f.Read((char*)&cx, sizeof(int));
	f.Read((char*)&cy, sizeof(int));
	f.Read((char*)&type, sizeof(int));
	GridData data{
		Margin + (Width % TileSize) / 2, Margin + (Height % TileSize) / 2, cx, cy, TileSize, type
	};
	makeGrid(data);
	
	glm::ivec2 bonus{};
	f.Read((char*)&bonus, sizeof(bonus));
	makeBonus(bonus.x, bonus.y);

	f.Read((char*)&Data.playerStart, sizeof(Data.playerStart));
	f.Read((char*)&Data.playerStart2, sizeof(Data.playerStart2));

	int eCount = 0;
	f.Read((char*)&eCount, sizeof(eCount));
	Data.emeraldCount = eCount;
	for (int i = 0; i < eCount; ++i) {
		glm::ivec2 loc;
		f.Read((char*)&loc, sizeof(loc));

		makeEmerald(loc.x, loc.y);
	}

	int gCount = 0;
	f.Read((char*)&gCount, sizeof(gCount));

	for (int i = 0; i < gCount; ++i) {
		glm::ivec2 loc;
		f.Read((char*)&loc, sizeof(loc));

		makeGoldBag(loc.x, loc.y);
	}

	int enCount = 3;
	f.Read((char*)&enCount, sizeof(enCount));
	Data.nogginCount = enCount;

	int pCount;
	std::vector<Direction> pathV;
	f.Read((char*)&pCount, sizeof(pCount));
	for (int i = 0; i < pCount; ++i) {
		int d = 0;
		f.Read((char*)&d, sizeof(d));
		pathV.push_back((Direction)d);
	}

	makeClearer(Data.playerStart.x, Data.playerStart.y, enCount, pathV);

	return Data;
}

void SaveLevel(const std::string& path)
{
	File::CreateFile(path);
	File f = File::OpenFile(path);
	if (!f) return;

	Grid* g = Grid::GetObject(0);
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	f.Write((char*)&g->Data.cells_x, sizeof(int));
	f.Write((char*)&g->Data.cells_y, sizeof(int));

	int type = 0;
	f.Write((char*)&type, sizeof(int));

	glm::ivec2 bonus{2, 10};
	f.Write((char*)&bonus, sizeof(bonus));
	
	auto ploc = PlayerComponent::GetObject(0)->GetOwner()->GetComponent<GridMoveComponent>()->GetGridLoc();
	f.Write((char*)&ploc, sizeof(ploc));
	ploc.y += 1;
	f.Write((char*)&ploc, sizeof(ploc));

	auto e = Scene->GetAllRootsOfType("emerald");
	int size = (int)e.size();
	f.Write((char*)&size, sizeof(size));

	for (auto& o : e) {
		glm::ivec2 loc;
		auto& pos = o->GetComponent<TransformComponent>()->GetPosition();
		loc.x = ((int)pos.x - g->Data.x) / g->Data.size;
		loc.y = ((int)pos.y - g->Data.y) / g->Data.size;

		f.Write((char*)&loc, sizeof(loc));
	}

	auto goldbag = Scene->GetAllRootsOfType("goldbag");
	size = (int)goldbag.size();
	f.Write((char*)&size, sizeof(size));

	for (auto& o : goldbag) {
		glm::ivec2 loc;
		loc = o->GetComponent<GridMoveComponent>()->GetGridLoc();

		f.Write((char*)&loc, sizeof(loc));
	}

	int enemyCount = 4;
	f.Write((char*)&enemyCount, sizeof(enemyCount));

	auto& pathc = PathClearer::GetObject(0)->GetPath();
	size = (int)pathc.size();
	f.Write((char*)&size, sizeof(size));
	for (auto& o : pathc) {
		int d = (int)o;
		f.Write((char*)&d, sizeof(d));
	}
}

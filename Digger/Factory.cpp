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
#include "File.h"
#include "Data.h"

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
	score->Init(player);
	trans->SetLocalPosition({ 10, 0, 0 });
	go2->SetParent(root);

	scene.Add(root);
}

void makePlayer(dae::User user, dae::Scene& scene, float speed, int x, int y)
{

	auto go1 = new GameObject("player");
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

	overlap->OnCollision.Bind(go1, [ref = gridmove->GetPermanentReference()](GameObject*, GameObject* other) {
		if (auto it = other->GetComponent<GoldBag>()) {
			if (ref->GetDirection() == Direction::Left || ref->GetDirection() == Direction::Right)
				it->Push(ref.Get(), ref->GetDirection());
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

void makeEnemy(int x, int y)
{

	auto go1 = new GameObject("enemy");
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

	overlap->SetRadius(15.f);
	sprite->SetTexture("VSBAG.gif");
	sprite->SetSize(glm::vec3{ SpriteSize });

	Scene->Add(go);
}

void makeClearer(int x, int y, const std::vector<Direction>& path)
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
	clear->ClearPath(path);

	/**/

	Scene->Add(go);
}

void makeSpawner(int x, int y)
{
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	auto go = new GameObject("spawner");

	auto spawn = CreateComponent<EnemySpawner>(go);
	spawn->StartSpawn(x, y);

	Scene->Add(go);
}

glm::ivec2 LoadLevel(const std::string& path)
{
	File f = File::OpenFile(path);
	if (!f) return { 0,0 };

	int cx = 0, cy = 0;
	int Margin = 20;
	int Width = 840 - Margin * 2;
	int Height = 480 - Margin * 2;
	f.Read((char*)&cx, sizeof(int));
	f.Read((char*)&cy, sizeof(int));
	GridData data{
		Margin + (Width % TileSize) / 2, Margin + (Height % TileSize) / 2, cx, cy, TileSize
	};
	makeGrid(data);
	
	glm::ivec2 ploc{};
	f.Read((char*)&ploc, sizeof(ploc));

	int eCount = 0;
	f.Read((char*)&eCount, sizeof(eCount));

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

	int pCount;
	std::vector<Direction> pathV;
	f.Read((char*)&pCount, sizeof(pCount));
	for (int i = 0; i < pCount; ++i) {
		int d = 0;
		f.Read((char*)&d, sizeof(d));
		pathV.push_back((Direction)d);
	}

	makeClearer(1, 1, pathV);

	return ploc;
}

void SaveLevel(const std::string& path)
{
	File f = File::CreateFile(path);
	if (!f) return;

	Grid* g = Grid::GetObject(0);
	auto Scene = SceneManager::GetInstance().GetCurrentScene();

	f.Write((char*)&g->Data.cells_x, sizeof(int));
	f.Write((char*)&g->Data.cells_y, sizeof(int));
	
	auto ploc = PlayerComponent::GetObject(0)->GetOwner()->GetComponent<GridMoveComponent>()->GetGridLoc();
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

	auto& pathc = PathClearer::GetObject(0)->GetPath();
	size = (int)pathc.size();
	f.Write((char*)&size, sizeof(size));
	for (auto& o : pathc) {
		int d = (int)o;
		f.Write((char*)&d, sizeof(d));
	}
}

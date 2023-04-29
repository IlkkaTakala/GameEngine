#if _DEBUG
// ReSharper disable once CppUnusedIncludeDirective
#if __has_include(<vld.h>)
#include <vld.h>
#endif
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "Renderer.h"
#include "TextComponent.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"
#include "InputComponent.h"
#include "UIComponent.h"
#include "EngineTime.h"
#include "Collider.h"
#include "EventHandler.h"
#include <format>
#include "Grid.h"

#include <fstream>
#include <iostream>

constexpr int TileSize{ 45 };
constexpr int SpriteSize{ int(TileSize * 0.9) };

namespace Events
{
	constexpr int PlayerDeath		= 100;
	constexpr int PlayerScoreGained = 101;
	constexpr int ScoreEmerald		= 121;
	constexpr int ScoreGold			= 122;
	constexpr int ScoreEnemy		= 123;
	constexpr int GoldBagCrush		= 102;
}

void makeGold(int x, int y);
void makeEnemy();
void makeEmerald();
void makeGoldBag();

namespace dae {

class GridMoveComponent : public BaseComponent
	{
		COMPONENT(GridMoveComponent)

	public:

		void OnCreated() override
		{
			Transform = GetOwner()->GetComponent<TransformComponent>();
			actual = Transform->GetPosition();
		}

		void Tick(float /*delta*/) override
		{
			Transform->SetLocalPosition(glm::vec3(actual, 0.0));
		}

		void Init(float s, Grid* gridC, int x = 0, int y = 0) {
			SetSpeed(s);
			SetDirection(Direction::None);
			gridComponent = gridC;
			maxProgress = (float)gridC->Data.size;
			actual = gridC->GetCellCenter(x, y);
			grid = glm::ivec2{ x, y };
			gridC->MoveInto(GetOwner(), x, y);
		}

		void SetSpeed(float s)
		{
			speed = s;
		}

		void SetCell(int x, int y) {
			auto gridC = gridComponent.Get();
			gridC->MoveOut(GetOwner(), grid.x, grid.y);
			actual = gridC->GetCellCenter(x, y);
			grid = glm::ivec2{ x, y };
			progress = 0.f;
			gridC->MoveInto(GetOwner(), x, y);
		}

		Direction GetDirection() const { return direction; }

		void SetDirection(Direction dir, bool force = false)
		{
			if (!force && IsInProgress()) return;
			direction = dir;
			OnDirectionChanged.Broadcast(dir);
			target = nullptr;
			targetGrid = nullptr;
		}

		bool Move(float delta)
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
				return true;
			}
			return false;
		}

		bool IsInProgress() const { return progress < maxProgress&& progress > maxProgress / 2; }

		glm::ivec2 GetGridLoc() const { return grid; }

		MulticastDelegate<Direction> OnDirectionChanged;
		MulticastDelegate<Grid*, Direction, int, int> OnGridChanged;
		MulticastDelegate<Grid*, Direction, int, int> OnMoved;
		std::function<bool(Grid*, Direction, int, int)> GetCanMove;

	private:
		void OnDestroyed() override {
			gridComponent->MoveOut(GetOwner(), grid.x, grid.y);
		}

		void UpdateDirection()
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

		ComponentRef<TransformComponent> Transform;

		ComponentRef<Grid> gridComponent;
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

class PlayerComponent : public BaseComponent
{
	COMPONENT(PlayerComponent);
public:

	void Init(User user) {
		UserID = user;
	}

	void TakeDamage() {
		if (Dead) return;
		Lives--;
		OnDamaged.Broadcast();
		emeraldStreak = 0;
		Dead = true;
		GetOwner()->GetComponent<GridMoveComponent>()->SetDirection(Direction::None, true);

		if (auto it = GetOwner()->GetComponent<InputComponent>(); it != nullptr) {
			it->SetInputEnabled(false);
		}
		GetOwner()->GetComponent<SpriteComponent>()->SetTexture("VGRAVE5.gif");
		Time::GetInstance().SetTimerByEvent(2.f, [this]() {
			if (auto it = GetOwner()->GetComponent<InputComponent>(); it != nullptr) {
				it->SetInputEnabled(true);
			}
			GetOwner()->GetComponent<GridMoveComponent>()->SetCell(1, 1);
			GetOwner()->GetComponent<SpriteComponent>()->SetTexture("digger.tga");
			Dead = false;
		});

		if (Lives <= 0) {
		}
	}

	bool IsAlive() { return Lives > 0; }

	void GiveScore(int amount) {
		if (Dead) return;
		Score += amount;
		OnScoreGained.Broadcast(Score);
		EventHandler::FireEvent(Events::PlayerScoreGained, GetOwner());
	}

	int GetLives() const { return Lives; }
	int GetScore() const { return Score; }

	User GetID() const { return UserID; }

	MulticastDelegate<> OnDeath;
	MulticastDelegate<> OnDamaged;
	MulticastDelegate<int> OnScoreGained;
private:

	void OnNotified(Event e) override {
		if (Dead) return;
		switch (e.type)
		{
		case Events::GoldBagCrush:
			TakeDamage();
			break;

		case Events::ScoreEmerald:
			e.object->Destroy();
			GiveScore(25);
			if (++emeraldStreak >= 8) {
				emeraldStreak = 0;
				GiveScore(250);
			}
			break;

		case Events::ScoreGold:
			e.object->Destroy();
			GiveScore(500);
			break;

		default:
			break;
		}
	}

	User UserID;

	int emeraldStreak{ 0 };

	bool Dead{ false };
	int Lives{ 3 };
	int Score{ 0 };
};

class LifeDisplay : public BaseComponent
{
	COMPONENT(LifeDisplay);
public:

	void Init(PlayerComponent* user) {
		Text = GetOwner()->GetComponent<TextComponent>();
		Text->SetText(std::format("Lives: {}", Lives));
		user->OnDamaged.Bind(GetOwner(), [Disp = GetPermanentReference()]() {
			Disp->Lives--;
			Disp->Text->SetText(std::format("Lives: {}", Disp->Lives));
		});
	}

private:
	ComponentRef<TextComponent> Text;
	int Lives{ 3 };
};

class ScoreDisplay : public BaseComponent
{
	COMPONENT(ScoreDisplay);

private:
	ComponentRef<TextComponent> Text;
public:

	void Init(PlayerComponent* user) {
		Text = GetOwner()->GetComponent<TextComponent>();
		Text->SetText(std::format("{:05}", 0));

		user->OnScoreGained.Bind(GetOwner(), [Text = Text](int score) {
			Text->SetText(std::format("{:05}", score));
		});
	}
};

class EnemyController : public BaseComponent
{
	COMPONENT(EnemyController);

private:
	ComponentRef<TransformComponent> Transform;
	float Speed{ 50.f };

public:

	void OnCreated() override {
		Transform = GetOwner()->GetComponent<TransformComponent>();
	}

	void Tick(float) override {
		float closest = 10000.f;
		PlayerComponent* player = nullptr;
		for (auto& p : PlayerComponent::__object_list()) {
			if (!p.IsValid()) continue;
			auto& t1 = p.GetOwner()->GetComponent<TransformComponent>()->GetPosition();
			auto& t2 = Transform->GetPosition();

			if (glm::distance(t1, t2) < closest) {
				closest = glm::distance(t1, t2);
				player = &p;
			}
		}
		if (player) {
			auto dir = glm::normalize(player->GetOwner()->GetComponent<TransformComponent>()->GetPosition() - Transform->GetPosition());
			Transform->SetLocalPosition(Transform->GetLocalPosition() + dir * Time::GetInstance().GetDelta() * Speed);
		}
	}
};

class GoldBag : public BaseComponent
{
	COMPONENT(GoldBag)

public:
	void StartMoving() {
		IsMoving = true;
	}

	void StopMoving() {
		IsMoving = false;
		if (fallDist >= 2) {
			GetOwner()->Destroy();
			auto g = GetOwner()->GetComponent<GridMoveComponent>()->GetGridLoc();
			makeGold(g.x, g.y);
		}
		fallDist = 0;
	}

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
}

void makeDisplay(dae::PlayerComponent* player, dae::Scene& scene)
{
	using namespace dae;

	auto root = new GameObject();
	auto trans = CreateComponent<TransformComponent>(root);
	trans->SetPosition({10, 10 + 85 * player->GetID(), 0});

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
	text->SetColor(SDL_Color{0, 200, 0, 255});
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

void makeEnemy()
{
	using namespace dae;

	auto go1 = new GameObject();
	SceneManager::GetInstance().GetCurrentScene()->Add(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);
	/*auto enemy =*/ CreateComponent<EnemyController>(go1);

	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		if (other->HasComponent<PlayerComponent>()) {
			makeEnemy();
			other->GetComponent<PlayerComponent>()->TakeDamage();
			self->Destroy();
		}
	});

	overlap->SetRadius(20.f);
	sprite->SetTexture("enemy.tga");
	trans->SetPosition({ 10 + rand() % 820, 10 + rand() % 460, 0 });
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
	grid->OnGridChanged.Bind(go, [ref = bag->GetPermanentReference()](Grid*, Direction, int, int) {
		ref->GridUpdated();
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

void begin(dae::Scene& scene)
{
	using namespace dae;
	int Margin = 20;
	int Width = 840 - Margin * 2;
	int Height = 480 - Margin * 2;
	GridData data{
		Margin + (Width % TileSize) / 2, Margin + (Height % TileSize) / 2, Width / TileSize, Height / TileSize, TileSize
	};
	makeGrid(data);

	MulticastDelegate<User>::DelegateHandle Connected = InputManager::GetInstance().OnUserDeviceConnected.Bind(nullptr, [&scene](User user) {
		printf("Player %d connected\n", user);
		bool found = false;
		for (auto& p : PlayerComponent::__object_list()) {
			if (p.GetID() == user) found = true;
		}
		if (!found) {
			makePlayer(user, scene, 300.f);
		}
	});
	for (auto& u : InputManager::GetInstance().GetActiveUsers()) {
		makePlayer(u, scene, 150.f);
	}
	makeEmerald();
	makeGoldBag();
	makeGoldBag();
	makeGoldBag();
}

void load()
{
	srand((unsigned int)time(NULL));
	using namespace dae;

	auto& scene = SceneManager::GetInstance().CreateScene("Demo");

	auto go4 = new GameObject();
	auto ui = CreateComponent<UIComponent>(go4);
	ui->BuildUI([](GameObject* owner) {
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
		ImGui::Begin("Controls", nullptr, flags);
		ImGui::Text("Gather as many emeralds as you can, but avoid the enemies");
		ImGui::Text("Use WASD, left controller stick or dpad to move");
		ImGui::Text("Multiple players can join by connecting new controllers");
		ImGui::Separator();
		ImGui::Text("One emerald gives 100 points and the victory achievement unlocks with 500 points");
		ImGui::Text("Enemies substract health but no round end is implemented");

		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = 80;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 100.f);
		if (ImGui::Button("Begin game", { 80, 30 })) {
			owner->Destroy();
			begin(*owner->GetScene());
		}

		ImGui::End();
	});
	scene.Add(go4);

	InputManager::GetInstance().OnUserDeviceDisconnected.Bind(nullptr, [](User user) {
		printf("Player %d disconnected\n", user);
	});
	
	// Create the default input mapping for the players.
	InputManager::GetInstance().MakeInputMapping("Default", 
		{
			{"Move",{
				{
					Key('w', DeviceType::Keyboard, InputMappingType::DigitalToY),		// WASD movement, 
					Key('a', DeviceType::Keyboard, InputMappingType::DigitalToNegX),
					Key('s', DeviceType::Keyboard, InputMappingType::DigitalToNegY),
					Key('d', DeviceType::Keyboard, InputMappingType::DigitalToX),
					Key(Buttons::Axis::ControllerStickLeft, DeviceType::Controller, InputMappingType::Axis2D),	// Movement using stick too
					Key(Buttons::Controller::DpadUp, DeviceType::Controller, InputMappingType::DigitalToY),		// Dpad movement
					Key(Buttons::Controller::DpadLeft, DeviceType::Controller, InputMappingType::DigitalToNegX),
					Key(Buttons::Controller::DpadDown, DeviceType::Controller, InputMappingType::DigitalToNegY),
					Key(Buttons::Controller::DpadRight, DeviceType::Controller, InputMappingType::DigitalToX),
				},
				true }
			},
		});
}

int main(int, char*[]) {

	dae::Minigin engine("../Data/");
	engine.Run(load);

	return 0;
}
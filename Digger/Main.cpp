#if _DEBUG
// ReSharper disable once CppUnusedIncludeDirective
#if __has_include(<vld.h>)
#include <vld.h>
#endif
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "BaseComponent.h"
#include "InputManager.h"
#include "Components.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Scene.h"
#include "Data.h"
#include "Delegates.h"
#include "Grid.h"
#include "Factory.h"
#include "UIComponent.h"
#include "GameState.h"

struct HallOfFame
{
	struct Player {
		char name[3];
		int score;
	} Players[10];
};

class State_Game : public dae::GameState
{
public:
	void Init() {
		using namespace dae;
		/*int Margin = 20;
		int Width = 840 - Margin * 2;
		int Height = 480 - Margin * 2;
		GridData data{
			Margin + (Width % TileSize) / 2, Margin + (Height % TileSize) / 2, Width / TileSize, Height / TileSize, TileSize
		};
		makeGrid(data);*/
		auto& scene = SceneManager::GetInstance().CreateScene("level");

		glm::ivec2 playerStart = LoadLevel("Lvel.lvl");
		MulticastDelegate<User>::DelegateHandle Connected = InputManager::GetInstance().OnUserDeviceConnected.Bind(nullptr, [&scene, playerStart](User user) {
			printf("Player %d connected\n", user);
			bool found = false;
			for (auto& p : PlayerComponent::ObjectList()) {
				if (p.GetID() == user) found = true;
			}
			if (!found) {
				makePlayer(user, scene, 300.f, playerStart.x, playerStart.y);
			}
			});
		for (auto& u : InputManager::GetInstance().GetActiveUsers()) {
			makePlayer(u, scene, 150.f, playerStart.x, playerStart.y);
		}

		//makeEmerald(rand() % (data.cells_x - 1), rand() % (data.cells_y - 1));
		//makeEmerald(rand() % (data.cells_x - 1), rand() % (data.cells_y - 1));
		//makeEmerald(rand() % (data.cells_x - 1), rand() % (data.cells_y - 1));
		//makeEmerald(rand() % (data.cells_x - 1), rand() % (data.cells_y - 1));
		//makeEmerald(rand() % (data.cells_x - 1), rand() % (data.cells_y - 1));
		//makeGoldBag(rand() % (data.cells_x - 1), rand() % (data.cells_y / 2));
		//makeGoldBag(rand() % (data.cells_x - 1), rand() % (data.cells_y / 2));
		//makeGoldBag(rand() % (data.cells_x - 1), rand() % (data.cells_y / 2));
		//makeClearer(1, 1, {
		//	Direction::Right,
		//		Direction::Right,
		//		Direction::Right,
		//		Direction::Right,
		//		Direction::Down,
		//		Direction::Down,
		//		Direction::Down,
		//		Direction::Down,
		//		Direction::Down,
		//		Direction::Right,
		//		Direction::Right,
		//		Direction::Right,
		//		Direction::Up,
		//		Direction::Up,
		//		Direction::Up,
		//		Direction::Up,
		//		Direction::Up,
		//		Direction::Up,
		//		Direction::Right,
		//		Direction::Right,
		//		Direction::Right,
		//		Direction::Right,
		//	});

		//SaveLevel("Lvel.lvl");
	}

	State_Game() : GameState("game") {}

	bool toMenu{ false };
};

class State_MainMenu : public dae::GameState
{

public:

	void Init() {
		using namespace dae;
		auto& scene = SceneManager::GetInstance().CreateScene("Demo");

		auto go4 = new GameObject("menu");
		auto ui = CreateComponent<UIComponent>(go4);
		ui->BuildUI([this](GameObject* owner) {
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
			ImGui::Begin("Controls", nullptr, flags);
			ImGui::Text("Gather as many emeralds as you can, but avoid the enemies");
			ImGui::Text("Use WASD, left controller stick or dpad to move");
			ImGui::Text("Multiple players can join by connecting new controllers");
			ImGui::Separator();
			ImGui::Text("One emerald gives 25 points and gold pile gives 500");
			ImGui::Text("Enemies substract health but no round end is implemented");
			ImGui::Separator();
			ImGui::Text("Enemies spawn in intervals at their spawn location");
			ImGui::Separator();
			ImGui::Text("Currently only collecting points, breaking bags, and dying have sounds");


			auto windowWidth = ImGui::GetWindowSize().x;
			auto textWidth = 80;

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 100.f);
			if (ImGui::Button("Begin game", { 80, 30 })) {
				owner->Destroy();
				toGame = true;
			}

			ImGui::End();
			});
		scene.Add(go4);

		InputManager::GetInstance().OnUserDeviceDisconnected.Bind(nullptr, [](User user) {
			printf("Player %d disconnected\n", user);
			});
	}

	void Exit() {
		dae::SceneManager::GetInstance().RemoveScene("Demo");
	}

	State_MainMenu() : GameState("mainmenu") {}

	bool toGame{false};
};

void load(dae::GameStateManager* state)
{
	srand((unsigned int)time(NULL));
	using namespace dae;

	// Create the default input mapping for the players.
	InputManager::GetInstance().MakeInputMapping("Default",
		{
			{"Spawn", {
				{
					Key('f')
				}, false
				}
			},
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

	auto mptr = new State_MainMenu();
	auto gptr = new State_Game();
	auto menu = state->AddState(mptr);
	auto game = state->AddState(gptr);

	state->AddPath(menu, game, [mptr](GameState*) -> bool {
		return mptr->toGame;
		});

	state->AddPath(game, menu, [gptr](GameState*) -> bool {
		return gptr->toMenu;
	});
}

int main(int, char*[]) {

	dae::Minigin engine("../Data/");
	engine.Run(load);

	return 0;
}
#if _DEBUG
// ReSharper disable once CppUnusedIncludeDirective
#if __has_include(<vld.h>)
#include <vld.h>
#endif
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "EventHandler.h"

#include "BaseComponent.h"
#include "Components.h"
#include "InputComponent.h"
#include "TransformComponent.h"
#include "UIComponent.h"

#include "GameObject.h"
#include "Scene.h"
#include "Data.h"
#include "Delegates.h"
#include "Grid.h"
#include "Factory.h"

#include "GameState.h"
#include "Highscores.h"
#include "GameGlobals.h"
#include <format>

class State_Game : public dae::GameState, public dae::EventListener
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
		Player = nullptr;
		Player2 = nullptr;
		auto& scene = SceneManager::GetInstance().CreateScene("persistent");
		toMenu = false;
		GameGlobals::GetInstance().SetScore(0);

		MulticastDelegate<User>::DelegateHandle Connected = InputManager::GetInstance().OnUserDeviceConnected.Bind(nullptr, [&scene](User user) {
			printf("Player %d connected\n", user);
			bool found = false;
			for (auto& p : PlayerComponent::ObjectList()) {
				if (p.GetID() == user) found = true;
			}
			});
		
		Player = makePlayer(0, scene, 150.f, 1, 1);
		switch (GameGlobals::GetInstance().GetType())
		{
		case GameType::Single: {
			Player->GetComponent<InputComponent>()->AddUser(1);
		} break;
		case GameType::Coop: {
			Player2 = makePlayer(1, scene, 150.f, 1, 1);
		} break;
		case GameType::Pvp: {
			
		} break;
		default:
			break;
		}
		Player->GetComponent<InputComponent>()->AddUser(2);
		makeHUD(Player, Player2);
		NextLevel();

		//std::vector<glm::ivec2> locs{
		//	{1,4},
		//	{2,4},
		//	{1,5},
		//	{2,5},
		//	{1,6},
		//	{2,6},

		//	{7,4},
		//	{7,5},

		//	{11,2},
		//	{12,2},
		//	{13,2},
		//	{11,3},
		//	{12,3},
		//	{13,3},

		//	{14,5},
		//	{15,5},
		//	{16,5},
		//	{14,6},
		//	{15,6},
		//	{16,6},
		//	{14,7},
		//	{15,7},
		//	{16,7},
		//};
		//std::vector<glm::ivec2> locs2{
		//	{7, 2},
		//	{15, 2},
		//	{3, 4},
		//};

		//for (auto& l : locs)
		//	makeEmerald(l.x, l.y);

		//for (auto& l : locs2)
		//	makeGoldBag(l.x, l.y);

		//makeClearer(1, 1, 4, {
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

		//SaveLevel("Level1.lvl");
	}

	void Exit() {
		dae::SceneManager::GetInstance().RemoveScene("level");
		dae::SceneManager::GetInstance().RemoveScene("persistent");
		toMenu = false;
	}

	State_Game() : GameState("game"), EventListener({Events::PlayerDeath, Events::EmeraldDestroy, Events::EnemyDestroy, Events::EnemySpawn, Events::SkipLevel}) {}

	void Notified(dae::EventType event, dae::GameObject* object) {
		switch (event) {
		case Events::PlayerDeath: {
			if (Player->GetComponent<PlayerComponent>()->GetLives() <= 0
			 && (!Player2 || Player2->GetComponent<PlayerComponent>()->GetLives() <= 0)) {
				toMenu = true;
			}
		} break;

		case Events::EmeraldDestroy: {
			CollectedEmeralds++;
			CheckEnd();
		} break;

		case Events::EnemyDestroy: {
			DestroyedEnemies++;
			CheckEnd();
		} break;

		case Events::SkipLevel: {
			NextLevel();
		} break;

		case Events::EnemySpawn: {
			if (GameGlobals::GetInstance().GetType() != GameType::Pvp || enemyPossessed) break;
			object->GetComponent<Enemy>()->Possess(1);
			enemyPossessed = true;
		} break;
		}
	}

	void NextLevel() {
		using namespace dae;
		if (loading) return;
		SceneManager::GetInstance().RemoveScene("level");
		Player->SetActive(false);
		if (Player2) Player2->SetActive(false);
		loading = true;
		enemyPossessed = false;

		Time::GetInstance().SetTimerByEvent(0.2f, [this] {
			SceneManager::GetInstance().CreateScene("level");
			if (++CurrentLevel >= LevelCount) CurrentLevel = 0;
			Data = LoadLevel(std::format("Level{}.lvl", CurrentLevel));
			Player->GetComponent<PlayerComponent>()->LevelChanged(Data.playerStart.x, Data.playerStart.y);
			if (Player2) Player2->GetComponent<PlayerComponent>()->LevelChanged(Data.playerStart2.x, Data.playerStart2.y);
			CollectedEmeralds = 0;
			DestroyedEnemies = 0;
			loading = false;
		});
	}

	void CheckEnd() {
		if (CollectedEmeralds >= Data.emeraldCount || DestroyedEnemies >= Data.nogginCount) {
			NextLevel();
		}
	}

	LevelData Data;
	int CollectedEmeralds;
	int DestroyedEnemies;

	bool enemyPossessed{ false };
	bool loading{ false };
	int CurrentLevel{};
	int LevelCount{ 1 };
	dae::GameObject* Player{};
	dae::GameObject* Player2{};
	bool toMenu{ false };
};

class State_MainMenu : public dae::GameState
{

public:

	void Init() {
		using namespace dae;
		using namespace ImGui;
		auto& scene = SceneManager::GetInstance().CreateScene("Demo");
		toGame = false;
		Type = GameType::None;

		auto go4 = new GameObject("menu");
		auto ui = CreateComponent<UIComponent>(go4);
		ui->BuildUI([this](GameObject* owner) {
			static ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
			auto windowSize = Renderer::GetInstance().GetWindowSize() - glm::ivec2{40, 60};
			SetNextWindowSize({ (float)windowSize.x, (float)windowSize.y }, ImGuiCond_Always);
			SetNextWindowPos(ImVec2{ 20.f, 40.f }, ImGuiCond_Always);
			Begin("Controls", nullptr, flags);
			auto windowWidth = GetWindowWidth();
			if (BeginTable("menu", 2)) {
				TableNextColumn();
				Text("Gather as many emeralds as you can, but avoid the enemies");
				Text("Use WASD, left controller stick or dpad to move");
				Text("Multiple players can join by connecting new controllers");
				Separator();
				Text("One emerald gives 25 points and gold pile gives 500");
				Text("Enemies substract health but no round end is implemented");
				Separator();
				Text("Enemies spawn in intervals at their spawn location");
				Separator();
				Text("Currently only collecting points, breaking bags, and dying have sounds");

				TableNextColumn();
				Text("Highscores");
				if (Button("Singleplayer", { 95, 25 })) {
					Highscore = GameType::Single;
				}
				SameLine();
				if (Button("Co-op", { 95, 25 })) {
					Highscore = GameType::Coop;
				}
				SameLine();
				if (Button("PvP", { 95, 25 })) {
					Highscore = GameType::Pvp;
				}
				if (BeginTable("highscores", 2))
				{
					
					auto& score = Highscores::GetInstance().GetScores();
					for (int row = 0; row < 10; row++)
					{
						if (score.Players[(int)Highscore][row].score == 0) break;
						TableNextRow();
						TableNextColumn();
						Text(score.Players[(int)Highscore][row].name);
						TableNextColumn();
						Text(std::to_string(score.Players[(int)Highscore][row].score).c_str());
					}
					EndTable();
				}
				EndTable();
			}

			switch (Type)
			{
			case GameType::None: {
				auto textWidth = 320;
				SetCursorPosX((windowWidth - textWidth) * 0.5f);
				SetCursorPosY(GetWindowSize().y - 100.f);
				if (Button("Singleplayer", { 100, 30 })) {
					Type = GameType::Single;
				}
				SameLine();
				if (Button("Co-op", { 100, 30 })) {
					Type = GameType::Coop;
				}
				SameLine();
				if (Button("PvP", { 100, 30 })) {
					Type = GameType::Pvp;
				}/*
				SetCursorPosX((windowWidth - 100) * 0.5f);
				if (Button("Exit", { 100, 30 })) {
				}*/
			} break;
			case GameType::Single: {
				SetCursorPosX((windowWidth - 300) * 0.5f);
				SetCursorPosY(GetWindowSize().y - 100.f);
				SetNextItemWidth(300.f);
				InputText("Name", GameGlobals::GetInstance().GetPlayerName(0), 4, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank);
				SetCursorPosX((windowWidth - 210) * 0.5f);
				if (Button("Start game", { 100, 30 })) {
					GameGlobals::GetInstance().SetType(GameType::Single);
					owner->Destroy();
					toGame = true;
				}
				SameLine();
				if (Button("Back", { 100, 30 })) {
					Type = GameType::None;
				}
			} break;
			case GameType::Coop:
				SetCursorPosX((windowWidth - 300) * 0.5f);
				SetCursorPosY(GetWindowSize().y - 100.f);
				SetNextItemWidth(300.f);
				InputText("First name", GameGlobals::GetInstance().GetPlayerName(0), 4, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank);
				SetCursorPosX((windowWidth - 300) * 0.5f);
				SetNextItemWidth(300.f);
				InputText("Second name", GameGlobals::GetInstance().GetPlayerName(1), 4, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank);
				SetCursorPosX((windowWidth - 210) * 0.5f);
				if (Button("Start game", { 100, 30 })) {
					GameGlobals::GetInstance().SetType(GameType::Coop);
					owner->Destroy();
					toGame = true;
				}
				SameLine();
				if (Button("Back", { 100, 30 })) {
					Type = GameType::None;
				}
				break;
			case GameType::Pvp:
				SetCursorPosX((windowWidth - 300) * 0.5f);
				SetCursorPosY(GetWindowSize().y - 100.f);
				SetNextItemWidth(300.f);
				InputText("Digger name", GameGlobals::GetInstance().GetPlayerName(0), 4, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank);
				SetCursorPosX((windowWidth - 300) * 0.5f);
				SetNextItemWidth(300.f);
				InputText("Noggin name", GameGlobals::GetInstance().GetPlayerName(1), 4, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank);
				SetCursorPosX((windowWidth - 210) * 0.5f);
				if (Button("Start game", { 100, 30 })) {
					GameGlobals::GetInstance().SetType(GameType::Pvp);
					owner->Destroy();
					toGame = true;
				}
				SameLine();
				if (Button("Back", { 100, 30 })) {
					Type = GameType::None;
				}
				break;
			default:
				break;
			}

			End();
			});
		scene.Add(go4);
	}

	void Exit() {
		dae::SceneManager::GetInstance().RemoveScene("Demo");
		toGame = false;
	}

	State_MainMenu() : GameState("mainmenu"), Type(GameType::None) {}

	GameType Highscore{GameType::Single};
	GameType Type;
	bool toGame{false};
};

class State_EndMenu : public dae::GameState, public dae::EventListener
{

public:

	void Init() {
		using namespace dae;
		auto& scene = SceneManager::GetInstance().CreateScene("End");
		toMenu = false;
		toGame = false;

		auto go4 = new GameObject("menu");
		auto ui = CreateComponent<UIComponent>(go4);
		ui->BuildUI([this](GameObject* owner) {
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
			ImGui::Begin("Game ended", nullptr, flags);
			ImGui::Text("The game has ended");
			ImGui::Separator();
			ImGui::Text("Final score");
			ImGui::Text("%d", GameGlobals::GetInstance().GetScore());


			auto windowWidth = ImGui::GetWindowSize().x;
			auto textWidth = 80;

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 150.f);
			if (ImGui::Button("New game", { 80, 30 })) {
				owner->Destroy();
				toGame = true;
			}
			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 100.f);
			if (ImGui::Button("Main menu", { 80, 30 })) {
				owner->Destroy();
				toMenu = true;
			}
			ImGui::End();
			});
		scene.Add(go4);
	}

	void Notified(dae::EventType e, dae::GameObject* o) override {
		if (e == Events::PlayerFinalScore) {
			auto player = o->GetComponent<PlayerComponent>();
			Scores.push_back(player->GetScore());

			Highscores::GetInstance().TryAddScore(player->GetName().c_str(), player->GetScore(), GameGlobals::GetInstance().GetType());
		}
	}

	void Exit() {
		dae::SceneManager::GetInstance().RemoveScene("End");
		toMenu = false;
		toGame = false;
	}

	State_EndMenu() : GameState("endmenu"), EventListener({Events::PlayerFinalScore}) {}

	bool toMenu{ false };
	bool toGame{ false };
private:
	std::vector<int> Scores;
};

void load(dae::StateManager* state)
{
	srand((unsigned int)time(NULL));
	using namespace dae;

	// Create the default input mapping for the players.
	InputManager::GetInstance().MakeInputMapping("None", {});
	InputManager::GetInstance().MakeInputMapping("Default",
		{
			{"Spawn", {
				{
					Key('f')
				}, false
				}
			},
			{"Mute", {
				{
					Key('m')
				}, false
				}
			},
			{"Skip", {
				{
					Key(SDLK_F1)
				}, false
				}
			},
			{"Fire", {
				{
					Key(' '),
					Key(Buttons::Controller::ButtonA, DeviceType::Controller),
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
	auto eptr = new State_EndMenu();
	auto menu = state->AddState(mptr);
	auto game = state->AddState(gptr);
	auto end = state->AddState(eptr);

	state->AddPath(menu, game, [mptr](GameState*) -> bool {
		return mptr->toGame;
		});

	state->AddPath(game, end, [gptr](GameState*) -> bool {
		return gptr->toMenu;
	});

	state->AddPath(end, menu, [eptr](GameState*) -> bool {
		return eptr->toMenu;
	});

	state->AddPath(end, game, [eptr](GameState*) -> bool {
		return eptr->toGame;
	});

	Highscores::GetInstance().Initialize();
}

int main(int, char*[]) {

	dae::Minigin engine("../Data/");
	engine.Run(load);

	return 0;
}
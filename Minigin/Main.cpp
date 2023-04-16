#include <SDL.h>

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
#include "Time.h"
#include "Collider.h"
#include "EventHandler.h"
#include <format>

#include "steam/steam_api.h"
#include <fstream>
#include <iostream>


namespace Events
{
	constexpr int PlayerDeath = 100;
	constexpr int PlayerScoreGained = 100;
}

namespace dae {
class PlayerComponent : public BaseComponent
{
	COMPONENT(PlayerComponent);
public:

	void Init(User user) {
		UserID = user;
	}

	void TakeDamage() {
		Lives--;
		OnDamaged.Broadcast();
		if (Lives <= 0) {
			OnDeath.Broadcast();
			EventHandler::FireEvent(Events::PlayerDeath, GetOwner());
		}
	}

	bool IsAlive() { return Lives > 0; }

	void GiveScore() {
		Score += 100;
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

	User UserID;

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
		Text->SetText(std::format("Score: {}", 0));

		user->OnScoreGained.Bind(GetOwner(), [Text = Text](int score) {
			Text->SetText(std::format("Score: {}", score));
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

#define _ACH_ID( id, name ) { id, #id, name, "", 0, 0 }
struct Achievement_t
{
	int m_eAchievementID;
	const char* m_pchAchievementID;
	char m_rgchName[128];
	char m_rgchDescription[256];
	bool m_bAchieved;
	int m_iIconImage;
};

// Defining our achievements
enum EAchievements
{
	ACH_WIN_ONE_GAME = 0,
};

// Achievement array which will hold data about the achievements and their state
Achievement_t g_Achievements[] =
{
	_ACH_ID(ACH_WIN_ONE_GAME, "Winner"),
};

class AchievementListener final : public Singleton<AchievementListener>, public EventListener
{
	
public:
	AchievementListener()
		: EventListener({
		Events::PlayerScoreGained,
		}), 
		m_iAppID(0),
		m_bInitialized(false),
		m_CallbackUserStatsReceived(this, &AchievementListener::OnUserStatsReceived),
		m_CallbackUserStatsStored(this, &AchievementListener::OnUserStatsStored),
		m_CallbackAchievementStored(this, &AchievementListener::OnAchievementStored)
	{
		m_iAppID = SteamUtils()->GetAppID();
		m_pAchievements = g_Achievements;
		m_iNumAchievements = 1;
		RequestStats();
	}
protected:

	void Notified(EventType event, GameObject* object) {
		switch (event)
		{
		case Events::PlayerScoreGained: {
			if (object->GetComponent<PlayerComponent>()->GetScore() >= 500) {
				printf("Player achieved victory\n");
				SetAchievement("ACH_WIN_ONE_GAME");
				RemoveEventType(Events::PlayerScoreGained);
			}
		} break;

		default:
			break;
		}
	}

private:

	bool SetAchievement(const char* ID)
	{
		// Have we received a call back from Steam yet?
		if (m_bInitialized)
		{
			SteamUserStats()->SetAchievement(ID);
			return SteamUserStats()->StoreStats();
		}
		// If not then we can't set achievements yet
		return false;
	}

	bool RequestStats()
	{
		// Is Steam loaded? If not we can't get stats.
		if (NULL == SteamUserStats() || NULL == SteamUser())
		{
			return false;
		}
		// Is the user logged on?  If not we can't get stats.
		if (!SteamUser()->BLoggedOn())
		{
			return false;
		}
		// Request user stats.
		return SteamUserStats()->RequestCurrentStats();
	}

	STEAM_CALLBACK(AchievementListener, OnUserStatsReceived, UserStatsReceived_t,
		m_CallbackUserStatsReceived)
	{
		// we may get callbacks for other games' stats arriving, ignore them
		if (m_iAppID == (int64)pParam->m_nGameID)
		{
			if (k_EResultOK == pParam->m_eResult)
			{
				printf("Received stats and achievements from Steam\n");
				m_bInitialized = true;

				// load achievements
				for (int iAch = 0; iAch < m_iNumAchievements; ++iAch)
				{
					Achievement_t& ach = m_pAchievements[iAch];

					SteamUserStats()->GetAchievement(ach.m_pchAchievementID, &ach.m_bAchieved);
					_snprintf(ach.m_rgchName, sizeof(ach.m_rgchName), "%s",
						SteamUserStats()->GetAchievementDisplayAttribute(ach.m_pchAchievementID,
							"name"));
					_snprintf(ach.m_rgchDescription, sizeof(ach.m_rgchDescription), "%s",
						SteamUserStats()->GetAchievementDisplayAttribute(ach.m_pchAchievementID,
							"desc"));

					if (ach.m_eAchievementID == ACH_WIN_ONE_GAME && ach.m_bAchieved == true) {
						RemoveEventType(Events::PlayerScoreGained);
					}
				}
			}
			else
			{
				char buffer[128];
				_snprintf(buffer, 128, "RequestStats - failed, %d\n", pParam->m_eResult);
				printf(buffer);
			}
		}
	}

	STEAM_CALLBACK(AchievementListener, OnUserStatsStored, UserStatsStored_t,
		m_CallbackUserStatsStored)
	{
		// we may get callbacks for other games' stats arriving, ignore them
		if (m_iAppID == (int64)pParam->m_nGameID)
		{
			if (k_EResultOK == pParam->m_eResult)
			{
				printf("Stored stats for Steam\n");
			}
			else
			{
				char buffer[128];
				_snprintf(buffer, 128, "StatsStored - failed, %d\n", pParam->m_eResult);
				printf(buffer);
			}
		}
	}

	STEAM_CALLBACK(AchievementListener, OnAchievementStored,
		UserAchievementStored_t, m_CallbackAchievementStored)
	{
		// we may get callbacks for other games' stats arriving, ignore them
		if (m_iAppID == (int64)pParam->m_nGameID)
		{
			printf("Stored Achievement for Steam\n");
		}
	}

	int64 m_iAppID; // Our current AppID
	Achievement_t* m_pAchievements; // Achievements data
	int m_iNumAchievements; // The number of Achievements
	bool m_bInitialized;
};

}

void makeDisplay(dae::PlayerComponent* player, dae::Scene& scene)
{
	using namespace dae;

	auto root = new GameObject();
	auto trans = CreateComponent<TransformComponent>(root);
	auto text = CreateComponent<TextComponent>(root);
	text->Init(std::format("Player {}: ", player->GetID()), ResourceManager::GetInstance().LoadFont("Lingua.otf", 25));
	trans->SetPosition({0, 10 + 85 * player->GetID(), 0});

	auto go1 = new GameObject();
	trans = CreateComponent<TransformComponent>(go1);
	text = CreateComponent<TextComponent>(go1);
	auto life = CreateComponent<LifeDisplay>(go1);
	text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
	life->Init(player);
	trans->SetLocalPosition({ 10, 28, 0 });
	go1->SetParent(root);

	auto go2 = new GameObject();
	trans = CreateComponent<TransformComponent>(go2);
	text = CreateComponent<TextComponent>(go2);
	auto score = CreateComponent<ScoreDisplay>(go2);
	text->Init(" ", ResourceManager::GetInstance().LoadFont("Lingua.otf", 20));
	score->Init(player);
	trans->SetLocalPosition({ 10, 56, 0 });
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

	overlap->SetRadius(12.f);
	sprite->SetTexture(user == 0 ? "digger.tga" : "digger2.tga");
	trans->SetPosition({ 120, 200 + 30 * user, 0 });
	input->SetUserFocus(user);
	InputManager::GetInstance().SetUserMapping(user, "Default");

	input->Bind2DAction("Move", [ref = go1->GetComponent<TransformComponent>()->GetPermanentReference(), speed](float x, float y) {
		auto transform = ref.Get<TransformComponent>();
		if (transform) transform->SetLocalPosition(transform->GetLocalPosition() + glm::vec3(x, -y, 0.f) * Time::GetInstance().GetDelta() * speed);
	});

	makeDisplay(player, scene);

	scene.Add(go1);
}

void makeEnemy(dae::Scene& scene)
{
	using namespace dae;

	auto go1 = new GameObject();
	scene.Add(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);
	/*auto enemy =*/ CreateComponent<EnemyController>(go1);

	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		if (other->HasComponent<PlayerComponent>()) {
			makeEnemy(*self->GetScene());
			other->GetComponent<PlayerComponent>()->TakeDamage();
			self->Destroy();
		}
	});

	overlap->SetRadius(20.f);
	sprite->SetTexture("enemy.tga");
	trans->SetPosition({ 10 + rand() % 820, 10 + rand() % 460, 0 });
}

void makePickup(dae::Scene& scene)
{
	using namespace dae;

	auto go1 = new GameObject();
	scene.Add(go1);
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	auto overlap = CreateComponent<SphereOverlap>(go1);

	overlap->OnCollision.Bind(go1, [](GameObject* self, GameObject* other) {
		if (other->HasComponent<PlayerComponent>()) {
			makePickup(*self->GetScene());
			other->GetComponent<PlayerComponent>()->GiveScore();
			self->Destroy();
		}
	});
	
	overlap->SetRadius(20.f);
	sprite->SetTexture("VEMERALD.tga");
	trans->SetPosition({ 10 + rand() % 820, 10 + rand() % 460, 0});
}

void begin(dae::Scene& scene)
{
	using namespace dae;


	MulticastDelegate<User>::DelegateHandle Connected = InputManager::GetInstance().OnUserDeviceConnected.Bind(nullptr, [&scene](User user) {
		printf("Player %d connected\n", user);
		bool found = false;
		for (auto& p : PlayerComponent::__object_list()) {
			if (p.GetID() == user) found = true;
		}
		if (!found) {
			makePlayer(user, scene, 300.f);
			makeEnemy(scene);
		}
	});
	for (auto& u : InputManager::GetInstance().GetActiveUsers()) {
		makePlayer(u, scene, 150.f);
		makeEnemy(scene);
	}
	makePickup(scene);
}

void load()
{
	srand((unsigned int)time(NULL));
	using namespace dae;

	auto& scene = SceneManager::GetInstance().CreateScene("Demo");
	auto go3 = new GameObject();
	auto font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto title = CreateComponent<TextComponent>(go3);
	auto trans = CreateComponent<TransformComponent>(go3);
	title->Init("Programming 4 Assignment", font);
	trans->SetPosition({ 150, 20, 0 });
	
	scene.Add(go3);

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
			{"Move",
				{
					Key(SDLK_w, DeviceType::Keyboard, InputMappingType::DigitalToY),		// WASD movement, 
					Key(SDLK_a, DeviceType::Keyboard, InputMappingType::DigitalToNegX),
					Key(SDLK_s, DeviceType::Keyboard, InputMappingType::DigitalToNegY),
					Key(SDLK_d, DeviceType::Keyboard, InputMappingType::DigitalToX),
					Key(Buttons::Axis::ControllerStickLeft, DeviceType::Controller, InputMappingType::Axis2D),	// Movement using stick too
					Key(Buttons::Controller::DpadUp, DeviceType::Controller, InputMappingType::DigitalToY),		// Dpad movement
					Key(Buttons::Controller::DpadLeft, DeviceType::Controller, InputMappingType::DigitalToNegX),
					Key(Buttons::Controller::DpadDown, DeviceType::Controller, InputMappingType::DigitalToNegY),
					Key(Buttons::Controller::DpadRight, DeviceType::Controller, InputMappingType::DigitalToX),
				}
			},
		});

	AchievementListener::GetInstance();
}

int main(int, char*[]) {

	if (!SteamAPI_Init())
	{
		std::cerr << "Fatal Error - Steam must be running to play this game (SteamAPI_Init() failed)." << std::endl;
		return 1;
	}
	else
		std::cout << "Successfully initialized steam." << std::endl;

	dae::Minigin engine("../Data/");
	engine.Run(load);

	SteamAPI_Shutdown();

	return 0;
}
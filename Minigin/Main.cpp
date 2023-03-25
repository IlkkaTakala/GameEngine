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
#include "Command.h"
#include <format>

#include <fstream>

void makePlayer(dae::User user, dae::Scene& scene, float speed)
{
	using namespace dae;

	auto go1 = new GameObject();
	auto trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	sprite->SetTexture(user == 0 ? "digger.tga" : "digger2.tga");
	auto input = CreateComponent<InputComponent>(go1);
	trans->SetPosition({ 120, 200 + 30 * user, 0 });
	input->SetUserFocus(user);
	auto ui = CreateComponent<UIComponent>(go1);
	ui->BuildUI([](GameObject* owner) {
		User player = owner->GetComponent<InputComponent>()->GetCurrentUser();
		auto& pos = owner->GetComponent<TransformComponent>()->GetPosition();
		ImGui::SetNextWindowPos({ 10.f, 10.f + 100.f * player }, ImGuiCond_Once);
		ImGui::Begin(("Player " + std::to_string(player)).c_str());

		ImGui::Text("Position: ");
		ImGui::Text(std::format("{:.0f}, {:.0f}", pos.x, pos.y).c_str());
		ImGui::End();
		});
	InputManager::GetInstance().SetUserMapping(user, "Default");

	input->BindAction("button1", [user]() {
		printf("Button down by player %d!\n", user);
	});

	auto disconnect = [go1, user](User user_disc) {
		if (user == user_disc) {
			go1->Destroy();
		}
	};

	auto Handle = InputManager::GetInstance().OnUserDeviceDisconnected.Bind(disconnect);

	input->Bind2DAction("Move", [ref = go1->GetComponent<TransformComponent>()->GetPermanentReference(), speed](float x, float y) {
		auto transform = ref.Get<TransformComponent>();
		transform->SetLocalPosition(transform->GetLocalPosition() + glm::vec3(x, -y, 0.f) * Time::GetInstance().GetDelta() * speed);
	});
	scene.Add(go1);
}

void load()
{
	using namespace dae;

	auto& scene = SceneManager::GetInstance().CreateScene("Demo");
	auto go3 = new GameObject();
	auto font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto title = CreateComponent<TextComponent>(go3);
	auto trans = CreateComponent<TransformComponent>(go3);
	title->Init("Programming 4 Assignment", font);
	trans->SetPosition({ 80, 20, 0 });
	
	scene.Add(go3);

	InputManager::GetInstance().OnUserDeviceConnected.Bind([&scene](User user) {
		printf("Player %d connected\n", user);
		makePlayer(user, scene, 300.f);
	});

	InputManager::GetInstance().OnUserDeviceDisconnected.Bind([](User user) {
		printf("Player %d disconnected\n", user);
	});
	// TODO: write enums for all keys
	InputManager::GetInstance().MakeInputMapping("Default", 
		{
			{"button1", 
				{
					Key(SDLK_SPACE, DeviceType::Keyboard, InputMappingType::Button, TriggerType::Down),	// Down trigger fires every frame when the button is down, triggers are only valid for buttons
					Key(0x1000, DeviceType::Controller, InputMappingType::Button, TriggerType::Down),	// A button
				}
			},
			{"Move",
				{
					Key(SDLK_w, DeviceType::Keyboard, InputMappingType::DigitalToY),		// WASD movement, 
					Key(SDLK_a, DeviceType::Keyboard, InputMappingType::DigitalToNegX),
					Key(SDLK_s, DeviceType::Keyboard, InputMappingType::DigitalToNegY),
					Key(SDLK_d, DeviceType::Keyboard, InputMappingType::DigitalToX),
					Key(0, DeviceType::Controller, InputMappingType::Axis2D),				// Movement using stick too
					Key(0x0001, DeviceType::Controller, InputMappingType::DigitalToY),		// Dpad movement
					Key(0x0004, DeviceType::Controller, InputMappingType::DigitalToNegX),
					Key(0x0002, DeviceType::Controller, InputMappingType::DigitalToNegY),
					Key(0x0008, DeviceType::Controller, InputMappingType::DigitalToX),
				}
			},
		});

	makePlayer(0, scene, 150.f);
}

int main(int, char*[]) {
	dae::Minigin engine("../Data/");
	engine.Run(load);
	return 0;
}
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
#include "Time.h"

#include <fstream>

float Time = 0.f;

void load()
{
	using namespace dae;

	Renderer::GetInstance().AddRenderSubsystem([]() {
		for (auto& o : SpriteComponent::__object_list()) {
			if (o.IsValid())
				o.Render();
		}
		for (auto& o : TextComponent::__object_list()) {
			if (o.IsValid())
				o.Render();
		}
	});

	auto& scene = SceneManager::GetInstance().CreateScene("Demo");
	auto go3 = new GameObject();
	auto font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto title = CreateComponent<TextComponent>(go3);
	auto trans = CreateComponent<TransformComponent>(go3);
	title->Init("Programming 4 Assignment", font);
	trans->SetPosition({ 80, 20, 0 });
	scene.Add(go3);


	auto go1 = new GameObject();
	trans = CreateComponent<TransformComponent>(go1);
	auto sprite = CreateComponent<SpriteComponent>(go1);
	sprite->SetTexture("digger.tga");
	auto input = CreateComponent<InputComponent>(go1);
	trans->SetPosition({ 80, 40, 0 });
	input->SetUserFocus(0);

	input->Bind2DAction(1, [ref = go1->GetComponent<TransformComponent>()->GetPermanentReference()](float x, float y) {
		auto transform = ref.Get<TransformComponent>();
		transform->SetLocalPosition(transform->GetLocalPosition() + glm::vec3(x, -y, 0.f) * Time::GetInstance().GetDelta() * 150.f);
	});

	input->SetInputEnabled(false);
}

int main(int, char*[]) {
	dae::Minigin engine("../Data/");
	engine.Run(load);
	return 0;
}
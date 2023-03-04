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
#include "FPSComponent.h"
#include "SpriteComponent.h"
#include "TransformComponent.h"

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

	auto go1 = std::make_shared<GameObject>();
	auto image = CreateComponent<SpriteComponent>(go1.get());
	image->SetTexture("background.tga");
	go1->AddComponent(image);
	scene.Add(go1);

	auto go2 = std::make_shared<GameObject>();
	auto logo = CreateComponent<SpriteComponent>(go2.get());
	auto trans = CreateComponent<TransformComponent>(go2.get());
	logo->SetTexture("logo.tga");
	trans->SetPosition(216, 180, 0);
	scene.Add(go2);

	auto go3 = std::make_shared<GameObject>();
	auto font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto title = CreateComponent<TextComponent>(go3.get());
	trans = CreateComponent<TransformComponent>(go3.get());
	title->Init("Programming 4 Assignment", font);
	trans->SetPosition(80, 20, 0);
	scene.Add(go3);

	auto go4 = std::make_shared<GameObject>();
	scene.Add(go4);

	font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 18);
	auto txt = CreateComponent<TextComponent>(go4.get());
	trans = CreateComponent<TransformComponent>(go4.get());
	/*auto fps = */CreateComponent<FPSComponent>(go4.get());
	trans->SetPosition(5, 5, 0);
	txt->Init("60.0", font);
	txt->SetColor({ 0, 255, 0 });


	auto first = std::make_shared<GameObject>();
	auto tra = CreateComponent<TransformComponent>();
	auto sprite = CreateComponent<SpriteComponent>();
	first->AddComponent(tra);
	first->AddComponent(sprite);
	scene.Add(first);

	first->AddTickSystem([](float) {
		
	});

	auto second = std::make_shared<GameObject>();
	tra = CreateComponent<TransformComponent>();
	sprite = CreateComponent<SpriteComponent>();
	second->AddComponent(tra);
	second->AddComponent(sprite);
	scene.Add(second);

	second->SetParent(first.get());

}

int main(int, char*[]) {
	dae::Minigin engine("../Data/");
	engine.Run(load);
    return 0;
}
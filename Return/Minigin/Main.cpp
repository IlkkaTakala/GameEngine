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

void load()
{
	dae::Renderer::GetInstance().AddRenderSubsystem([]() {
		for (auto& o : dae::SpriteComponent::__object_list()) {
			if (o.IsValid())
				o.Render();
		}
		for (auto& o : dae::TextComponent::__object_list()) {
			if (o.IsValid())
				o.Render();
		}
	});

	auto& scene = dae::SceneManager::GetInstance().CreateScene("Demo");

	auto go1 = std::make_shared<dae::GameObject>();
	auto image = CreateComponent<dae::SpriteComponent>();
	image->SetTexture("background.tga");
	go1->AddComponent(image);
	scene.Add(go1);

	auto go2 = std::make_shared<dae::GameObject>();
	auto logo = CreateComponent<dae::SpriteComponent>();
	auto trans = CreateComponent<dae::TransformComponent>();
	logo->SetTexture("logo.tga");
	trans->SetPosition(216, 180, 0);
	go2->AddComponent(logo);
	go2->AddComponent(trans);
	scene.Add(go2);

	auto go3 = std::make_shared<dae::GameObject>();
	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto title = CreateComponent<dae::TextComponent>();
	trans = CreateComponent<dae::TransformComponent>();
	title->Init("Programming 4 Assignment", font);
	trans->SetPosition(80, 20, 0);
	go3->AddComponent(title);
	go3->AddComponent(trans);
	scene.Add(go3);

	auto go4 = std::make_shared<dae::GameObject>();
	scene.Add(go4);

	font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 18);
	auto txt = CreateComponent<dae::TextComponent>();
	trans = CreateComponent<dae::TransformComponent>();
	auto fps = CreateComponent<dae::FPSComponent>();
	trans->SetPosition(5, 5, 0);
	txt->Init("60.0", font);
	txt->SetColor({ 0, 255, 0 });

	go4->AddComponent(txt);
	go4->AddComponent(trans);
	go4->AddComponent(fps);
}

int main(int, char*[]) {
	dae::Minigin engine("../Data/");
	engine.Run(load);
    return 0;
}
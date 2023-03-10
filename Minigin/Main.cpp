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
#include "PointRotatorComponent.h"
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

	auto go1 = new GameObject();
	auto image = CreateComponent<SpriteComponent>(go1);
	image->SetTexture("background.tga");
	go1->AddComponent(image);
	scene.Add(go1);

	auto go2 = new GameObject();
	auto logo = CreateComponent<SpriteComponent>(go2);
	auto trans = CreateComponent<TransformComponent>(go2);
	logo->SetTexture("logo.tga");
	trans->SetPosition(216, 180, 0);
	scene.Add(go2);

	auto go3 = new GameObject();
	auto font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto title = CreateComponent<TextComponent>(go3);
	trans = CreateComponent<TransformComponent>(go3);
	title->Init("Programming 4 Assignment", font);
	trans->SetPosition(80, 20, 0);
	scene.Add(go3);

	auto go4 = new GameObject();
	scene.Add(go4);

	font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 18);
	auto txt = CreateComponent<TextComponent>(go4);
	trans = CreateComponent<TransformComponent>(go4);
	/*auto fps = */CreateComponent<FPSComponent>(go4);
	trans->SetPosition(5, 5, 0);
	txt->Init("60.0", font);
	txt->SetColor({ 0, 255, 0 });


	auto first = new GameObject();
	auto tra = CreateComponent<TransformComponent>(first);
	auto sprite = CreateComponent<SpriteComponent>(first);
	auto point = CreateComponent<PointRotatorComponent>(first);
	sprite->SetTexture("digger.tga");
	tra->SetPosition(250, 200, 0);
	point->Init(tra->GetLocalPosition(), glm::vec3{ 50.f, 0.f, 0.f }, 2.f);
	scene.Add(first);

	auto second = new GameObject();
	tra = CreateComponent<TransformComponent>(second);
	sprite = CreateComponent<SpriteComponent>(second);
	point = CreateComponent<PointRotatorComponent>(second);
	tra->SetLocalPosition(40, 0, 0);
	sprite->SetTexture("digger2.tga");
	point->Init(glm::vec3{ 0.f }, tra->GetLocalPosition(), -3.f);
	scene.Add(second);

	second->SetParent(first);

	Time::GetInstance().SetTimerByEvent(5.f, [second]() {
		second->Destroy();
	});

	Time::GetInstance().SetTimerByEvent(10.f, [first]() {
		first->Destroy();
	});
}

struct Vector
{
	float x, y, z;
};

int main(int, char*[]) {
	dae::Minigin engine("../Data/");
	engine.Run(load);
	return 0;
}
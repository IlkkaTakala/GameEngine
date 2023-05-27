#include "SceneManager.h"
#include "Scene.h"
#include "EngineTime.h"

void dae::SceneManager::Update(float delta)
{
	for(auto& scene : m_scenes)
	{
		scene->Update(delta);
	}
}

void dae::SceneManager::Render()
{
	for (const auto& scene : m_scenes)
	{
		scene->Render();
	}
}

dae::Scene& dae::SceneManager::CreateScene(const std::string& name)
{
	const auto& scene = std::shared_ptr<Scene>(new Scene(name));
	m_scenes.push_back(scene);
	return *scene;
}

void dae::SceneManager::RemoveScene(const std::string& name)
{
	for (auto& s : m_scenes) {
		if (s->GetName() == name) {
			Time::GetInstance().ClearAllTimers();
			s->RemoveAll();
			DeleteList.push_back(s);
		}
	}
}

dae::Scene* dae::SceneManager::GetCurrentScene()
{
	return m_scenes.empty() ? nullptr : m_scenes.back().get();
}

void dae::SceneManager::ClearScenes()
{
	for (auto& s : DeleteList) {
		std::erase(m_scenes, s);
	}
	DeleteList.clear();
}

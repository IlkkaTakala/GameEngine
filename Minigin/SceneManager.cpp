#include "SceneManager.h"
#include "Scene.h"

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
	std::erase_if(m_scenes, [name](auto& s) {
		return s->GetName() == name;
	});
}

dae::Scene* dae::SceneManager::GetCurrentScene()
{
	return m_scenes.empty() ? nullptr : m_scenes.back().get();
}
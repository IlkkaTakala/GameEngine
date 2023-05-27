#include "Scene.h"
#include "GameObject.h"

using namespace dae;

unsigned int Scene::m_idCounter = 0;

Scene::Scene(const std::string& name) : m_name(name) {}

Scene::~Scene() {

}

void Scene::Add(GameObject* object)
{
	if (object->MarkedForDelete) return;
	m_objects.emplace_back(std::move(object));
	object->SceneRef = this;
}

void Scene::Remove(GameObject* object, bool destroy)
{
	auto it = std::find(m_objects.begin(), m_objects.end(), object);
	if (it != m_objects.end()) {
		if (destroy) (*it)->Destroy();
		m_objects.erase(it);
	}
}

void Scene::RemoveAll()
{
	for (auto& o : m_objects) {
		o->Destroy();
	}
}

void Scene::Update(float delta)
{
	for(size_t i = 0; i < m_objects.size(); i++)
	{
		m_objects[i]->Update(delta);
	}
}

void Scene::Render() const
{
	/*for (const auto& object : m_objects)
	{
		object->Render();
	}*/
}

std::vector<GameObject*> dae::Scene::GetAllRootsOfType(const std::string& type)
{
	std::vector<GameObject*> vector;
	size_t hash = std::hash<std::string>{}(type);
	for (auto& o : m_objects) {
		if (o->Type == hash) {
			vector.push_back(o);
		}
	}
	return std::move(vector);
}


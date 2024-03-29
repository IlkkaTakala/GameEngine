#pragma once
#include "SceneManager.h"

namespace dae
{
	class GameObject;
	class Scene final
	{
		friend Scene& SceneManager::CreateScene(const std::string& name);
	public:
		void Add(GameObject* object);
		void Remove(GameObject* object, bool destroy = true);
		void RemoveAll();
		std::string GetName() const { return m_name; }

		void Update(float delta);
		void Render() const;
		std::vector<GameObject*> GetAllRootsOfType(const std::string& type);

		~Scene();
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

	private: 
		explicit Scene(const std::string& name);

		std::string m_name;
		std::vector<GameObject*> m_objects{};

		static unsigned int m_idCounter; 
	};

}

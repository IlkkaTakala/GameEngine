#pragma once
#include <memory>
#include <map>
#include <list>
#include <functional>
#include "ComponentRef.h"
#include "Scene.h"
#include "EventHandler.h"
#include "Delegates.h"

namespace dae
{
	struct AttachRules 
	{
		bool KeepWorldTransform{ true };
	};

	class BaseComponent;
	class GameObject final
	{
	public:
		virtual void Update(float delta);

		GameObject(const char* type);
		virtual ~GameObject();
		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

		void AddComponent(BaseComponent* Component);
		void AddTickSystem(const std::function<void(GameObject*, float)>& system);

		void Destroy();
		static void ForceCleanObjects();
		static void DeleteMarked();
		
		template<class T>
		bool RemoveComponent() {
			bool removed = false;
			for (auto& [type, c] : Components) {
				if (type == T::StaticType()) {
					c.Get()->Destroy();
					removed = true;
				}
			}
			return removed;
		}

		template <class T>
		T* GetComponent() {
			if (auto it = Components.find(T::StaticType()); it != Components.end()) {
				return it->second.Get<T>();
			}
			else return nullptr;
		}

		template<class T>
		bool HasComponent() const {
			auto it = Components.find(T::StaticType());
			return it != Components.end();
		}

		bool IsType(const std::string& type);
		bool HasComponent(const BaseComponent* component) const;

		void SetParent(GameObject* parent, AttachRules rules = AttachRules());
		GameObject* GetParent() const { return Parent; }
		std::list<GameObject*>& GetChildren() { return Children; }
		Scene* GetScene() const { return SceneRef; }

		void Notify(EventType event, GameObject* sender);
		MulticastDelegate<Event> OnNotified;

		void SetActive(bool active);
		bool IsActive() const { return isActive; }

	private:
		friend class Scene;

		void RemoveChild(GameObject* child);
		void AddChild(GameObject* child);

		std::map<int, ComponentRef<BaseComponent>> Components;

		Scene* SceneRef;
		GameObject* Parent{ nullptr };
		std::list<GameObject*> Children;
		std::vector<std::function<void(GameObject*, float)>> TickSystems;

		bool MarkedForDelete{ false };
		bool isActive{ true };

		size_t Type;
	};
}

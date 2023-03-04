#pragma once
#include <memory>
#include <list>
#include "BaseComponent.h"
#include "TransformComponent.h"

namespace dae
{
	class GameObject final
	{
	public:
		virtual void Update(float delta);

		GameObject() = default;
		virtual ~GameObject();
		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

		void AddComponent(BaseComponent* Component);
		
		/*
			Remove component from the game object
			Returns true if component was removed
		*/
		template<class T>
		bool RemoveComponent() {
			size_t removed = m_components.remove_if([](const ComponentRef<BaseComponent>& c) { return c.Type() == T::StaticType(); });
			return removed > 0;
		}

		template <class T>
		T* GetComponent() {
			for (auto& c : m_components) {
				if (c.Type() == T::StaticType()) {
					return &c.Get<T>();
				}
			}
			return nullptr;
		}

		template<class T>
		bool HasComponent() const {
			for (auto& c : m_components) {
				if (c.Type() == T::StaticType()) {
					return true;
				}
			}
			return false;
		}

		bool HasComponent(const BaseComponent* component) const {
			for (auto& c : m_components) {
				if (c.Type() == component->GetType()) {
					return true;
				}
			}
			return false;
		}

	private:

		std::list<ComponentRef<BaseComponent>> m_components;
	};
}

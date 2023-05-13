#pragma once
#include <map>
#include <string>
#include <vector>
#include <queue>

#include "Renderer.h"
#include "ComponentRef.h"
#include "GameObject.h"

namespace dae {
	
	template <class T>
	T* CreateComponent(GameObject* Owner = nullptr) {
		T n{};
		n.__set_type(T::StaticType());
		T* c_ptr = T::__add_component(std::move(n));
		if (Owner) Owner->AddComponent(c_ptr);
		c_ptr->OnCreated();
		return c_ptr;
	}

#define SET_RENDER_PRIORITY(PRIORITY) public: inline static constexpr int RenderPriority = PRIORITY;

class GameObject;
class BaseComponent
{
private:
	friend class GameObject;
	template <class T>
	friend class ComponentRef; 
	template <class T>
	friend T* CreateComponent(GameObject* Owner);

	GameObject* owner{ nullptr };

	void InternalTick(float delta) {
		if (!pendingDestroy) Tick(delta);
	}
	void __set_type(ComponentType t) { type = t; }

protected:
	bool alive{ false };
	bool pendingDestroy{ false };
	int type{ 0 };
	size_t id{ 0 };

	static int next() noexcept {
		static int componentCounter = 0;
		return componentCounter++;
	}

	static std::map<int, std::function<void(float)>>& GetTicks() {
		static std::map<int, std::function<void(float)>> Ticks;
		return Ticks;
	}

	static auto& __object_map() {
		static std::map<int, BaseComponent*> IdMap;
		return IdMap;
	}

	virtual BaseComponent* __get_object_as_base(size_t id_t) = 0;
	virtual int& ObjectList_counter_virtual() = 0;
	virtual void __clean_deleted() = 0;

	virtual void OnCreated() {}
	virtual void OnDestroyed() {}
	virtual void OnTreeBeginChange(AttachRules /*rules*/) {}
	virtual void OnTreeChanged(AttachRules /*rules*/) {}
	virtual void OnNotified(Event /*event*/) {}

public:
	BaseComponent() {}
	virtual ~BaseComponent() {}
	BaseComponent(const BaseComponent& other) = delete;
	BaseComponent(BaseComponent&& other) = default;
	BaseComponent& operator=(const BaseComponent& other) = delete;
	BaseComponent& operator=(BaseComponent&& other) = default;

	virtual void Tick(float /*delta*/) {}
	virtual void ComponentUpdate(float /*delta*/) {}
	void Destroy();
	static void CleanDestroyed();
	static void UpdateComponents(float delta) {
		for (auto& [type, t] : GetTicks()) {
			t(delta);
		}
	}

	bool IsValid() const;
	GameObject* GetOwner() const { return owner; }
	ComponentType GetType() const { return type; }
	ComponentRef<BaseComponent> GetPermanentReference() { return ComponentRef<BaseComponent>{ id, type }; }

	// TODO: Will clear the default objects, first add proper garbage collection
	/*static void ClearDefaultObjects() {
		for (auto& [type, ptr] : __object_map()) {
			delete ptr;
		}
	}*/
};

template<class T>
bool GetRegisterStatus() {
	static bool reg = Component<T>::init_component();
	return true;
}


/*
	No more macros.
	Not complete yet, but works in the same way as the previous macro
*/
template <class T>
class Component : public BaseComponent
{

private: 
	template <class T>
	friend T* CreateComponent(GameObject* Owner);
	template <class T>
	friend bool GetRegisterStatus();

	friend class ComponentRef<T>;
	friend class ComponentRef<BaseComponent>;

protected:
	explicit Component() {

	}
public:
	virtual ~Component() {}
	Component(const Component& other) = delete;
	Component(Component&& other) = default;
	Component& operator=(const Component& other) = delete;
	Component& operator=(Component&& other) = default;
public: 
	static ComponentType StaticType() {
		static int type_id = BaseComponent::next();
		return type_id; 
	} 
	static auto& ObjectList() { 
		static std::vector<T> Objects; 
		return Objects; 
	} 
private: 
	static int& ObjectList_counter() { 
		static int Counter; 
		return Counter; 
	} 
	virtual BaseComponent* __get_object_as_base(size_t id_t) {
		if (ObjectList().size() <= id_t) return nullptr;
		return dynamic_cast<BaseComponent*>(&ObjectList()[id_t]);
	}
	virtual int& ObjectList_counter_virtual() { 
		return ObjectList_counter(); 
	} 
	
	static auto& __free_list() { 
		static std::queue<size_t> free; 
		return free; 
	} 

	static T* __add_component(T&& c) { 
		if (__free_list().empty()) {
			c.id = ObjectList().size();
			if (c.id >= ObjectList().capacity()) 
				ObjectList_counter()++; 
			ObjectList().push_back(std::move(c)); 
			return &*ObjectList().rbegin();
		}
		else 
		{ 
			size_t id_t = __free_list().front();
			c.id = id_t; 
			ObjectList()[c.id] = std::move(c);
			__free_list().pop();
			return &ObjectList()[id_t]; 
		} 
	}

	void __remove_component() { 
		__free_list().push(id);
	}

	static bool init_component() {
		BaseComponent::__object_map().emplace(T::StaticType(), new T());
		Component<T>::ObjectList().reserve(50);
		if constexpr (!std::is_same_v<decltype(&T::ComponentUpdate), decltype(&BaseComponent::ComponentUpdate)>) {
			GetTicks().emplace(T::StaticType(), [](float delta) {
				for (auto& c : T::ObjectList()) {
					if (c.alive == true)
						c.ComponentUpdate(delta);
				}
				});
		}
		constexpr bool has_render = requires(T & t) {
			t.Render();
		};
		constexpr bool has_prio = requires(T & t) {
			t.RenderPriority;
		};

		if constexpr (has_render) {
			int prio = 1;
			if constexpr (has_prio) prio = T::RenderPriority;
			dae::Renderer::GetInstance().MakeRenderable<T>(prio);
		}
		return true;
	}

	inline static bool Registered = GetRegisterStatus<T>();

	virtual void __clean_deleted() {
		for (auto& c : ObjectList()) {
			if (c.pendingDestroy) {
				c.alive = false;
				c.pendingDestroy = false;
				c.__remove_component();
			}
		}
	}

public: 
	static T* GetObject(size_t id_t) { 
		return &ObjectList()[id_t]; 
	} 
	ComponentRef<T> GetPermanentReference() { return ComponentRef<T>{ id, type }; }
	ComponentRef<T> Ref() { GetPermanentReference(); }
};

}
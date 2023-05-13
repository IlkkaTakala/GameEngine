#pragma once
#include <map>
#include <string>
#include <vector>
#include <queue>

#include "Renderer.h"
#include "ComponentRef.h"
#include "GameObject.h"

namespace dae {
	
	/*
	This does some automated things to make class comparisons possible without casts.
	Also manages ECS and references, and gives possiblitity to refer to a class by string.
	Variables and vectors have static lifespan and will be destroyed when the program quits.

	TODO: Use templates instead
	*/


	template <class T>
	T* CreateComponent(GameObject* Owner = nullptr) {
		T n{};
		n.__set_type(T::StaticType());
		T* c_ptr = T::__add_component(std::move(n));
		if (Owner) Owner->AddComponent(c_ptr);
		c_ptr->OnCreated();
		return c_ptr;
	}

#define COMPONENT(CLASS) private: inline static bool registered = init_component<CLASS>(#CLASS);\
	\
	template <class T>\
	friend T* dae::CreateComponent(dae::GameObject* Owner);\
public: static dae::ComponentType StaticType() {\
	static int type_id = BaseComponent::__id_map()[#CLASS];\
	return type_id; \
} \
public: static auto& __object_list() { \
	static std::vector<CLASS> Objects; \
	return Objects; \
} \
public: static int& __object_list_counter() { \
	static int Counter; \
	return Counter; \
} \
private: virtual int& __object_list_counter_virtual() { \
	return __object_list_counter(); \
} \
public: virtual dae::BaseComponent* __get_object_as_base(size_t id_t) { \
	if (__object_list().size() <= id_t) return nullptr; \
	return dynamic_cast<dae::BaseComponent*>(&__object_list()[id_t]);\
}\
private: static auto& __free_list() { \
	static std::queue<size_t> free; \
	return free; \
} \
private: static CLASS* __add_component(CLASS&& c) { \
	if (__free_list().empty()) {\
		c.id = __object_list().size();\
		if (c.id >= __object_list().capacity()) \
			__object_list_counter()++; \
		__object_list().push_back(std::move(c)); \
		return &*__object_list().rbegin();\
	}\
	else \
	{ \
		size_t id_t = __free_list().front();\
		c.id = id_t; \
		__object_list()[c.id] = std::move(c);\
		__free_list().pop();\
		return &__object_list()[id_t]; \
	} \
}\
private: virtual void __remove_component() { \
	__free_list().push(id);\
}\
protected: virtual void __clean_deleted() {	\
	for (auto& c : __object_list()) { \
		if (c.pendingDestroy) { \
			c.alive = false;\
			c.pendingDestroy = false;\
			c.__remove_component();\
		}\
	} \
} \
public: static CLASS* GetObject(size_t id_t) { \
	return &__object_list()[id_t]; \
} \
dae::ComponentRef<CLASS> GetPermanentReference() { return dae::ComponentRef<CLASS>{ id, type }; }\
private:\

//#define ENABLE_RENDERING(CLASS) private: inline static bool Renderable = dae::Renderer::GetInstance().MakeRenderable<CLASS>(1);
//#define ENABLE_RENDERING_PRIORITY(CLASS, PRIORITY) private: inline static bool Renderable = dae::Renderer::GetInstance().MakeRenderable<CLASS>(PRIORITY);
#define SET_RENDER_PRIORITY(PRIORITY) public: inline static constexpr int RenderPriority = PRIORITY;

class GameObject;
class BaseComponent
{
private:
	friend class GameObject;
	template <class T>
	friend class ComponentRef;

	static int componentCount;
	GameObject* owner{ nullptr };

	void InternalTick(float delta) {
		if (!pendingDestroy) Tick(delta);
	}

	static std::map<int, std::function<void(float)>>& GetTicks() {
		static std::map<int, std::function<void(float)>> Ticks;
		return Ticks;
	}

protected:
	bool alive{ false };
	bool pendingDestroy{ false };
	int type{ 0 };
	size_t id{ 0 };

	static auto& __id_map() {
		static std::map<std::string, int> IdMap;
		return IdMap;
	}

	template <class T>
	static bool init_component(const char* name) {
		int type_id = BaseComponent::componentCount++;
		BaseComponent::__id_map().emplace(name, type_id);
		BaseComponent::__object_map().emplace(type_id, new T());
		T::__object_list().reserve(50);
		if constexpr (!std::is_same_v<decltype(&T::ComponentUpdate), decltype(&BaseComponent::ComponentUpdate)>) {
			GetTicks().emplace(type_id, [](float delta) {
				for (auto& c : T::__object_list()) {
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

	static auto& __object_map() {
		static std::map<int, BaseComponent*> IdMap;
		return IdMap;
	}

	void __set_type(ComponentType t) { type = t; }

	COMPONENT(BaseComponent);

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

	// TODO: Will clear the default objects, first add proper garbage collection
	/*static void ClearDefaultObjects() {
		for (auto& [type, ptr] : __object_map()) {
			delete ptr;
		}
	}*/
};

}
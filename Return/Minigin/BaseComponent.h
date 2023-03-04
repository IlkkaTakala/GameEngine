#pragma once
#include <map>
#include <string>
#include <vector>
#include <queue>

namespace dae {

	typedef int ComponentType;

	template <class T>
	class ComponentRef
	{
		size_t ID;
		ComponentType type;

	public:
		ComponentRef(size_t id, ComponentType type) : ID(id), type(type) {}

		ComponentType Type() const { return type; }

		template <class G>
		G& Get() const {
			return G::GetObject(ID);
		}

		T& Get() const {
			return *T::__object_map()[type]->__get_object_as_base(ID);
		}

		friend bool operator==(const ComponentRef<T>& lhs, const ComponentRef<T>& rhs) {
			return (lhs.ID == rhs.ID && lhs.type == rhs.type);
		}
	};
	/*
	This does some automated things to make class comparisons possible without casts.
	Also manages ECS and references.
	*/
#define COMPONENT(CLASS) private: inline static bool registered = init_component<CLASS>(#CLASS);\
	\
public: static ComponentType StaticType() {\
	static int type_id = BaseComponent::__id_map()[#CLASS];\
	return type_id; \
} \
public: static auto& __object_list() { \
	static std::vector<CLASS> Objects; \
	return Objects; \
} \
public: virtual BaseComponent* __get_object_as_base(size_t id_t) { \
	return dynamic_cast<BaseComponent*>(&__object_list()[id_t]);\
}\
private: static auto& __free_list() { \
	static std::queue<size_t> free; \
	return free; \
} \
public: static CLASS* __add_component(CLASS&& c) { \
	if (__free_list().empty()) {\
		c.id = __object_list().size();\
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
public: static CLASS& GetObject(size_t id_t) { \
	return __object_list()[id_t]; \
} \
public: ComponentRef<CLASS> GetPermanentReference() { return ComponentRef<CLASS>{ id, type }; } \
private:\

class GameObject;
class BaseComponent
{
private:
	static int componentCount;
	GameObject* owner{ nullptr };
	bool alive{ false };

protected:
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
		return true;
	}

	COMPONENT(BaseComponent);

	virtual void OnCreated() {}
	virtual void OnDestroyed() {}


public:
	virtual void Tick(float /*delta*/) {}
	void Destroy();

	bool IsValid() const;
	GameObject* GetOwner() const { return owner; }
	void SetOwner(GameObject* go) { owner = go; alive = true; }
	ComponentType GetType() const { return type; }

	/* Internal only, placeholder. These are not supposed to be public */
	void __set_type(ComponentType t) { type = t; }
	static auto& __object_map() {
		static std::map<int, BaseComponent*> IdMap;
		return IdMap;
	}
};

}

template <class T>
T* CreateComponent() {
	T n{};
	n.__set_type(T::StaticType());
	return T::__add_component(std::move(n));
}
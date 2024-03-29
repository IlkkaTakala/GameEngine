#pragma once

namespace dae {

	typedef int ComponentType;
	class BaseComponent;

	template <class T = BaseComponent>
	class ComponentRef
	{
		long long ID;
		ComponentType type;
		mutable T* ptr{ nullptr };
		mutable int check{ -1 };

	public:
		ComponentRef() : ID(0), type(-1) {}
		ComponentRef(size_t id, ComponentType type) : ID(id), type(type) {}
		ComponentRef(T* newPtr) {
			if (newPtr) {
				ID = newPtr->id;
				type = newPtr->type;
			}
			else 
				ID = -1;

			ptr = newPtr;
			check = T::ObjectList_counter();
		}

		ComponentType Type() const { return type; }

		template <class G>
		G* Get() const {
			if (G::ObjectList_counter() == check) return (G*)ptr;
			if (G::StaticType() != type) return nullptr;
			ptr = (T*)G::GetObject((size_t)ID);
			check = G::ObjectList_counter();
			return (G*)ptr;
		}

		T* Get() const 
		{
			auto t = BaseComponent::__object_map()[type];
			if (t->ObjectList_counter_virtual() == check) return ptr;
			ptr = (T*)t->__get_object_as_base((size_t)ID);
			check = t->ObjectList_counter_virtual();
			return ptr;
		}

		friend bool operator==(const ComponentRef& lhs, const ComponentRef& rhs) {
			return (lhs.ID == rhs.ID && lhs.type == rhs.type);
		}

		bool IsValid() const {
			return ID != -1;
		}

		friend bool operator==(const void*& lhs, const ComponentRef& rhs) {
			if (rhs.ptr == nullptr) {
				return lhs == rhs.Get();
			}
			return lhs == rhs.ptr;
		}

		T* operator->() const {
			return Get();
		}
	};

}

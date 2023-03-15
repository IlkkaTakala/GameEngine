#pragma once

namespace dae {

	typedef int ComponentType;
	class BaseComponent;

	class ComponentRef
	{
		size_t ID;
		ComponentType type;
		mutable BaseComponent* ptr{ nullptr };
		mutable int check{ -1 };

	public:
		ComponentRef(size_t id, ComponentType type) : ID(id), type(type) {}

		ComponentType Type() const { return type; }

		template <class G>
		G* Get() const {
			if (G::__object_list_counter() == check) return (G*)ptr;
			if (G::StaticType() != type) return nullptr;
			ptr = (BaseComponent*)G::GetObject(ID);
			check = G::__object_list_counter();
			return (G*)ptr;
		}

		BaseComponent* Get() const;

		friend bool operator==(const ComponentRef& lhs, const ComponentRef& rhs) {
			return (lhs.ID == rhs.ID && lhs.type == rhs.type);
		}

		bool IsValid() {
			return ID != -1;
		}
	};

}

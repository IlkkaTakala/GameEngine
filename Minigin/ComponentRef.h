#pragma once

namespace dae {

	typedef int ComponentType;
	class BaseComponent;

	class ComponentRef
	{
		size_t ID;
		ComponentType type;

	public:
		ComponentRef(size_t id, ComponentType type) : ID(id), type(type) {}

		ComponentType Type() const { return type; }

		template <class G>
		G* Get() const {
			if (G::StaticType() != type) return nullptr;
			return G::GetObject(ID);
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

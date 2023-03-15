#include "ComponentRef.h"
#include "BaseComponent.h"

dae::BaseComponent* dae::ComponentRef::Get() const
{
	auto t = BaseComponent::__object_map()[type];
	if (t->__object_list_counter() == check) return ptr;
	ptr = t->__get_object_as_base(ID);
	check = t->__object_list_counter();
	return ptr;
}
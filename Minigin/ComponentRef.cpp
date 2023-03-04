#include "ComponentRef.h"
#include "BaseComponent.h"

dae::BaseComponent* dae::ComponentRef::Get() const
{
	return BaseComponent::__object_map()[type]->__get_object_as_base(ID);
}
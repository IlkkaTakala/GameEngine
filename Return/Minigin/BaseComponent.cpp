#include "BaseComponent.h"
#include "GameObject.h"

int dae::BaseComponent::componentCount = 0;

void dae::BaseComponent::Destroy()
{
	OnDestroyed();
	alive = false;
	__remove_component();
}

bool dae::BaseComponent::IsValid() const
{
	return alive;
}

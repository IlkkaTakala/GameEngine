#include "BaseComponent.h"
#include "GameObject.h"

int dae::BaseComponent::componentCount = 0;

void dae::BaseComponent::Destroy()
{
	OnDestroyed();
	pendingDestroy = true;
}

bool dae::BaseComponent::IsValid() const
{
	return alive && !pendingDestroy && owner;
}

void dae::BaseComponent::CleanDestroyed()
{
	for (auto& [id, c] : __object_map()) {
		c->__clean_deleted();
	}
}

#include "BaseComponent.h"
#include "GameObject.h"

void dae::BaseComponent::Destroy()
{
	if (!pendingDestroy) OnDestroyed();
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

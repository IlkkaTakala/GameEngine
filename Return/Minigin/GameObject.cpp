#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "BaseComponent.h"

dae::GameObject::~GameObject() = default;

void dae::GameObject::AddComponent(BaseComponent* Component)
{
	if (HasComponent(Component)) return;

	Component->SetOwner(this);
	m_components.push_back(Component->GetPermanentReference());
}

void dae::GameObject::Update(float delta) {
	for (auto& c : m_components) {
		c.Get().Tick(delta);
	}
}


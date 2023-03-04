#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "BaseComponent.h"

dae::GameObject::~GameObject() 
{
	for (auto& [type, c] : Components) {
		auto ptr = c.Get();
		if (ptr) ptr->Destroy();
	}
}

void dae::GameObject::AddComponent(BaseComponent* Component)
{
	if (HasComponent(Component)) return;

	Component->owner = this;
	Components.emplace(Component->GetType(), Component->GetPermanentReference());
}

void dae::GameObject::AddTickSystem(const std::function<void(GameObject*, float)>& system)
{
	TickSystems.push_back(system);
}

bool dae::GameObject::HasComponent(const BaseComponent* component) const
{
	auto it = Components.find(component->GetType());
	return it != Components.end();
}

void dae::GameObject::Update(float delta) {

	for (auto& s : TickSystems) {
		s(this, delta);
	}
	for (auto& [type, c] : Components) {
		c.Get()->Tick(delta);
	}
}

void dae::GameObject::SetParent(GameObject* parent, bool /*keepRelative*/) {

	if (Parent) Parent->RemoveChild(this);
	Parent = parent;
	
	for (auto& [type, c] : Components) {
		c.Get()->OnTreeChanged();
	}

}

void dae::GameObject::RemoveChild(GameObject* child)
{
	Children.remove(child);
}
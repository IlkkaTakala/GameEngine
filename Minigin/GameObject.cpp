#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "BaseComponent.h"

namespace dae {
	static std::list<GameObject*> ObjectList;
}

dae::GameObject::GameObject()
{
	ObjectList.push_back(this);
}

dae::GameObject::~GameObject()
{
	for (auto& [type, c] : Components) {
		auto ptr = c.Get();
		if (ptr) ptr->Destroy();
	}
}

void dae::GameObject::AddComponent(BaseComponent* Component)
{
	if (MarkedForDelete) return;
	if (HasComponent(Component)) return;

	Component->owner = this;
	Component->alive = true;
	Components.emplace(Component->GetType(), Component->GetPermanentReference());
}

void dae::GameObject::AddTickSystem(const std::function<void(GameObject*, float)>& system)
{
	TickSystems.push_back(system);
}

void dae::GameObject::Destroy()
{
	if (MarkedForDelete) return;
	MarkedForDelete = true;
	if (SceneRef) SceneRef->Remove(this, false);
	for (auto it = Children.begin(); it != Children.end(); it = Children.begin()) {
		(*it)->Destroy();
	}
	
	if (Parent) Parent->RemoveChild(this);
}

void dae::GameObject::ForceCleanObjects()
{
	for (auto& o : ObjectList) {
		o->Destroy();
	}
	for (auto& o : ObjectList) {
		delete o;
	}
	ObjectList.clear();
}

void dae::GameObject::DeleteMarked() 
{
	for (auto& o : ObjectList) {
		if (o && o->MarkedForDelete) {
			o->Children.clear();
			delete o;
			o = nullptr;
		}
	}
	ObjectList.remove(nullptr);
}

bool dae::GameObject::HasComponent(const BaseComponent* component) const
{
	auto it = Components.find(component->GetType());
	return it != Components.end();
}

void dae::GameObject::Update(float delta) {
	if (MarkedForDelete) return;
	for (auto& s : TickSystems) {
		s(this, delta);
	}
	for (auto& [type, c] : Components) {
		c.Get()->Tick(delta);
	}
	for (auto& o : Children) {
		o->Update(delta);
	}
}

void dae::GameObject::SetParent(GameObject* parent, AttachRules rules) {
	if (MarkedForDelete) return;
	if (Parent) Parent->RemoveChild(this);
	Parent = parent;
	
	if (Parent) Parent->AddChild(this);
	for (auto& [type, c] : Components) {
		c.Get()->OnTreeChanged(rules);
	}

}

void dae::GameObject::RemoveChild(GameObject* child)
{
	Children.remove(child);
	SceneRef->Add(child);
}

void dae::GameObject::AddChild(GameObject* child) {
	Children.push_back(child);
	SceneRef->Remove(child, false);
}

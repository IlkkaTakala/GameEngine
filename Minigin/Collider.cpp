#include "Collider.h"
#include "TransformComponent.h"

void dae::SphereOverlap::OnCreated()
{
}

void dae::SphereOverlap::ComponentUpdate(float /*delta*/)
{
	if (!IsValid()) return;
	for (auto& o : __object_list()) {
		if (!o.IsValid()) continue;
		if (&o == this) continue;
		auto t1 = this->GetOwner()->GetComponent<TransformComponent>();
		auto t2 = o.GetOwner()->GetComponent<TransformComponent>();
		
		if (glm::length(t1->GetPosition() - t2->GetPosition()) < o.Radius + Radius) {
			OnCollision.Broadcast(GetOwner(), o.GetOwner());
		}
	}
}

void dae::SphereOverlap::SetRadius(float radius)
{
	Radius = radius;
}

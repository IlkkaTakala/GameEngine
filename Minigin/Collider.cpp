#include "Collider.h"
#include "TransformComponent.h"

void dae::SphereOverlap::OnCreated()
{
}

void dae::SphereOverlap::ComponentUpdate(float /*delta*/)
{
	if (!IsValid() || !isActive) return;
	auto t1 = this->GetOwner()->GetComponent<TransformComponent>();
	for (auto& o : ObjectList()) {
		if (!o.IsValid() || !o.isActive) continue;
		if (&o == this) continue;
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

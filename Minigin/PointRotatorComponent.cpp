#include "PointRotatorComponent.h"
#include "TransformComponent.h"

void dae::PointRotatorComponent::Tick(float delta)
{
	TotalTime += delta * SpeedMultiplier;
	if (TotalTime > 10000.f) TotalTime -= 10000.f;

	auto trans = GetOwner()->GetComponent<TransformComponent>();
	if (trans) {
		trans->SetLocalPosition({
			Origin.x + Original.x * cos(TotalTime) - Original.y * sin(TotalTime),
			Origin.y + Original.x * sin(TotalTime) - Original.y * cos(TotalTime),
			0 }
		);
	}
}

void dae::PointRotatorComponent::Init(const glm::vec3& Point, const glm::vec3& Offset, float Speed)
{
	Original = Offset;
	SpeedMultiplier = Speed;
	Origin = Point;
}

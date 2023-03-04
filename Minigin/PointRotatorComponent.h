#pragma once
#include "BaseComponent.h"
#include "glm/glm.hpp"

namespace dae {

class PointRotatorComponent : public BaseComponent
{
	COMPONENT(PointRotatorComponent)

public:

	void Tick(float delta) override;

	void Init(const glm::vec3& Point, const glm::vec3& Offset, float Speed = 1.f);
	
private:

	float SpeedMultiplier{ 1.f };
	float TotalTime{ 0.f };
	glm::vec3 Origin{ 0.f };
	glm::vec3 Original{ 0.f };
};

}

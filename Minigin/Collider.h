#pragma once
#include "BaseComponent.h"
#include "Delegates.h"

namespace dae 
{

class SphereOverlap : public BaseComponent
{
	COMPONENT(SphereOverlap)

public:

	void OnCreated() override;

	void ComponentUpdate(float delta) override;
	void SetRadius(float radius);

	MulticastDelegate<GameObject*, GameObject*> OnCollision;

private:

	float Radius{ 0.f };

};

}

#pragma once
#include "BaseComponent.h"

namespace dae {

class FPSComponent : public BaseComponent
{
	COMPONENT(FPSComponent)

public:

	virtual void Tick(float delta) override;
};

}

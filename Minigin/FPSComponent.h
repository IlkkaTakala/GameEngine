#pragma once
#include "BaseComponent.h"

namespace dae {

class FPSComponent : public Component<FPSComponent>
{

public:

	virtual void Tick(float delta) override;
};

}

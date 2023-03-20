#pragma once
#include "BaseComponent.h"

namespace dae {

class UIComponent : public BaseComponent
{
	COMPONENT(UIComponent)

public:

	virtual void Render() {}

};

}
#pragma once
#include "Renderer.h"
#include "BaseComponent.h"
#include "imgui.h"

namespace dae {

class UIComponent : public Component<UIComponent>
{
	SET_RENDER_PRIORITY(2);
public:

	void Render();

	void BuildUI(const std::function<void(GameObject*)>& func) {
		UI = func;
	}

private:

	std::function<void(GameObject*)> UI;
};

}
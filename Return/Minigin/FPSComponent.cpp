#include "FPSComponent.h"
#include "TextComponent.h"
#include "GameObject.h"
#include <sstream>
#include <format>

void dae::FPSComponent::Tick(float delta)
{
	if (auto o = GetOwner(); o) {
		if (auto c = o->GetComponent<TextComponent>()) {
			c->SetText(std::format("FPS: {:.2f}", 1.f / delta));
		}
	}
}

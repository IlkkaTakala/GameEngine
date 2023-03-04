#pragma once
#include <memory>
#include "BaseComponent.h"

namespace dae {

class Texture2D;
class SpriteComponent : public BaseComponent
{
	COMPONENT(SpriteComponent)

public:

	void Render();

	void SetTexture(const std::string& filename);

private:

	std::shared_ptr<Texture2D> m_texture;
};

}

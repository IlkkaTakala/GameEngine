#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "BaseComponent.h"
#include "Renderer.h"

namespace dae {

class Texture2D;
class SpriteComponent : public Component<SpriteComponent>
{
public:

	void Render();

	void SetTexture(const std::string& filename);
	void SetSize(const glm::vec2& size);
	void SetSize(float size) { SetSize(glm::vec2{size}); }

private:

	std::shared_ptr<Texture2D> m_texture;
	glm::ivec2 m_size{ 0,0 };
};

}

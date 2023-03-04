#include "SpriteComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "GameObject.h"

void dae::SpriteComponent::Render()
{
	glm::vec3 pos{ 0 };
	if (auto t = GetOwner()->GetComponent<TransformComponent>()) {
		pos = t->GetPosition();
	}
	Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
}

void dae::SpriteComponent::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

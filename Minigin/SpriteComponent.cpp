#include "SpriteComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Texture2D.h"

void dae::SpriteComponent::Render()
{
	if (!m_texture) return;
	glm::vec3 pos{ 0 };
	float rot = 0.f;
	bool flip = false;
	if (auto t = GetOwner()->GetComponent<TransformComponent>()) {
		pos = t->GetPosition();
		rot = t->GetRotation();
		flip = t->GetScale().x < 0.f;
	}
	Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y, (float)m_size.x, (float)m_size.y, true, rot, flip);
}

void dae::SpriteComponent::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
	if (m_size.x == 0 && m_size.y == 0)
		SDL_QueryTexture(m_texture->GetSDLTexture(), nullptr, nullptr, &m_size.x, &m_size.y);
	SDL_SetTextureBlendMode(m_texture->GetSDLTexture(), SDL_BLENDMODE_BLEND);
}

void dae::SpriteComponent::SetSize(const glm::vec2& size)
{
	m_size = size;
}

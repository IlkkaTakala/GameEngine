#pragma once
#include <glm/glm.hpp>
#include "BaseComponent.h"

namespace dae
{
	class TransformComponent final : public BaseComponent
	{
		COMPONENT(TransformComponent)

	public:
		const glm::vec3& GetPosition() const { return m_position; }
		void SetPosition(float x, float y, float z);
		const float& GetRotation() const { return m_rotation; }
		void SetRotation(float r);
		const glm::vec3& GetScale() const { return m_scale; }
		void SetScale(float x, float y, float z);

	private:
		glm::vec3 m_position{ 0 };
		float m_rotation{ 0.f };
		glm::vec3 m_scale{ 0 };
	};
}

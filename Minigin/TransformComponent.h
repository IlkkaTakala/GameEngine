#pragma once
#include <glm/glm.hpp>
#include "BaseComponent.h"

namespace dae
{
	class TransformComponent final : public BaseComponent
	{
		COMPONENT(TransformComponent)

	public:
		const glm::vec3& GetPosition();
		void SetPosition(float x, float y, float z);
		const float& GetRotation();
		void SetRotation(float r);
		const glm::vec3& GetScale();
		void SetScale(float x, float y, float z);

		const glm::vec3& GetLocalPosition() const { return Position; }
		void SetLocalPosition(float x, float y, float z);
		const float& GetLocalRotation() const { return Rotation; }
		void SetLocalRotation(float r);
		const glm::vec3& GetLocalScale() const { return Scale; }
		void SetLocalScale(float x, float y, float z);

		const glm::mat4& GetWorldTransform();
		const glm::mat4& GetLocalTransform();

	private:

		void MarkDirty();
		void Recalculate();

		const glm::mat4& GetToRoot();

		glm::vec3 Position{ 0.f };
		float Rotation{ 0.f };
		glm::vec3 Scale{ 1.f };

		glm::vec3 WorldPosition{ 0.f };
		float WorldRotation{ 0.f };
		glm::vec3 WorldScale{ 1.f };

		glm::mat4 ToWorld{ 1.f };
		glm::mat4 ToRoot{ 1.f };
		glm::mat4 ToParent{ 1.f };


		bool LocationDirty{ false };

	};
}

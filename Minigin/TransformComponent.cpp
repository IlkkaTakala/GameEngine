#include "TransformComponent.h"

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/gtx/transform.hpp>
#pragma warning(disable : 6001)
#include <glm/gtx/matrix_decompose.hpp>
#pragma warning(pop)

const glm::vec3& dae::TransformComponent::GetPosition()
{
	if (LocationDirty) Recalculate();

	return WorldPosition;
}

void dae::TransformComponent::SetPosition(const glm::vec3& pos)
{
	glm::vec3 loc(pos.x, pos.y, pos.z);

	glm::vec3 origin(0.f);
	auto parent = GetOwner()->GetParent();
	TransformComponent* Transform = nullptr;
	if (parent) {
		Transform = parent->GetComponent<TransformComponent>();
	}

	if (Transform) {
		origin = Transform->GetPosition();
	}

	glm::vec3 res = loc - origin;
	SetLocalPosition(res);
}

const float& dae::TransformComponent::GetRotation()
{
	if (LocationDirty) Recalculate();

	return WorldRotation;
}

void dae::TransformComponent::SetRotation(float r)
{
	float origin(0.f);
	auto parent = GetOwner()->GetParent();
	TransformComponent* Transform = nullptr;
	if (parent) {
		Transform = parent->GetComponent<TransformComponent>();
	}

	if (Transform) {
		origin = Transform->GetRotation();
	}

	float res = r - origin;
	SetLocalRotation(res);
}

const glm::vec3& dae::TransformComponent::GetScale()
{
	if (LocationDirty) Recalculate();

	return WorldScale;
}

void dae::TransformComponent::SetScale(const glm::vec3& scale)
{
	glm::vec3 loc(scale.x, scale.y, scale.z);

	glm::vec3 origin(0.f);
	auto parent = GetOwner()->GetParent();
	TransformComponent* Transform = nullptr;
	if (parent) {
		Transform = parent->GetComponent<TransformComponent>();
	}

	if (Transform) {
		origin = Transform->GetScale();
	}

	glm::vec3 res = loc - origin;
	SetLocalScale(res);
}

void dae::TransformComponent::SetLocalPosition(const glm::vec3& pos)
{
	Position = pos;

	MarkDirty();
}

void dae::TransformComponent::SetLocalRotation(float r)
{
	Rotation = r;

	MarkDirty();
}

void dae::TransformComponent::SetLocalScale(const glm::vec3& scale)
{
	Scale = scale;

	MarkDirty();
}

const glm::mat4& dae::TransformComponent::GetWorldTransform()
{
	if (LocationDirty) Recalculate();

	return ToWorld;
}

const glm::mat4& dae::TransformComponent::GetLocalTransform()
{
	if (LocationDirty) Recalculate();

	return ToParent;
}

void dae::TransformComponent::OnTreeBeginChange(AttachRules rules)
{
	if (rules.KeepWorldTransform) {
		SetLocalPosition(GetPosition());
		SetLocalRotation(GetRotation());
		SetLocalScale(GetScale());
	}
}

void dae::TransformComponent::OnTreeChanged(AttachRules /*rules*/)
{
	MarkDirty();
}

void dae::TransformComponent::MarkDirty()
{
	LocationDirty = true;

	auto& children = GetOwner()->GetChildren();
	for (auto& c : children) {
		if (auto tc = c->GetComponent<TransformComponent>()) {
			tc->MarkDirty();
		}
	}
}

void dae::TransformComponent::Recalculate()
{
	auto parent = GetOwner()->GetParent();
	TransformComponent* Transform = nullptr;
	if (parent) {
		Transform = parent->GetComponent<TransformComponent>();
	}

	if (Transform) {
		ToRoot = Transform->GetWorldTransform();
	}
	else {
		ToRoot = glm::mat4(1.f);
	}

	ToParent = glm::translate(glm::mat4(1.0f), Position)
		* glm::rotate(Rotation, glm::vec3{ 0, 0, 1 })
		* glm::scale(glm::mat4(1.0f), Scale);

	ToWorld = ToRoot * ToParent;

	glm::quat Rot;
	glm::vec3 Skew;
	glm::vec4 Persp;
	glm::decompose(ToWorld, WorldScale, Rot, WorldPosition, Skew, Persp);

	LocationDirty = false;
}

const glm::mat4& dae::TransformComponent::GetToRoot()
{
	if (LocationDirty) Recalculate();

	return ToRoot;
}

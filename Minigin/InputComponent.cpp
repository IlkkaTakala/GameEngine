#include "BaseComponent.h"
#include "InputComponent.h"

void dae::InputComponent::SetUserFocus(User user)
{
	InputManager::GetInstance().UnregisterInputComponent(this);

	CurrentUser = user;

	InputManager::GetInstance().RegisterInputComponent(this, user);
}

void dae::InputComponent::SetInputEnabled(bool enabled)
{
	ReceivesInput = enabled;
}

void dae::InputComponent::Bind2DAction(int action, std::function<void(float, float)> callback)
{
	Actions2D.emplace(action, callback);
}

void dae::InputComponent::Trigger2DAction(int action, float x, float y)
{
	if (auto it = Actions2D.find(action); it != Actions2D.end()) {
		(*it).second(x, y);
	}
}

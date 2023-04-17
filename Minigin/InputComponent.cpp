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

void dae::InputComponent::OnDestroyed()
{
	InputManager::GetInstance().UnregisterInputComponent(this);
}

void dae::InputComponent::BindAction(const std::string& action, std::function<void()> callback)
{
	int ID = InputManager::GetInstance().ActionToActionID(action);
	if (ID != -1)
		Actions.emplace(ID, callback);
}

void dae::InputComponent::BindAxisAction(const std::string& action, std::function<void(float)> callback)
{
	int ID = InputManager::GetInstance().ActionToActionID(action);
	if (ID != -1)
		ActionsAxis.emplace(ID, callback);
}

void dae::InputComponent::Bind2DAction(const std::string& action, std::function<void(float, float)> callback)
{
	int ID = InputManager::GetInstance().ActionToActionID(action);
	if (ID != -1)
		Actions2D.emplace(ID, callback);
}

void dae::InputComponent::TriggerAction(int action)
{
	if (!ReceivesInput) return;
	if (auto it = Actions.find(action); it != Actions.end()) {
		(*it).second();
	}
}

void dae::InputComponent::Trigger2DAction(int action, float x, float y)
{
	if (!ReceivesInput) return;
	if (auto it = Actions2D.find(action); it != Actions2D.end()) {
		(*it).second(x, y);
	}
}

void dae::InputComponent::TriggerAxisAction(int action, float value)
{
	if (!ReceivesInput) return;
	if (auto it = ActionsAxis.find(action); it != ActionsAxis.end()) {
		(*it).second(value);
	}
}

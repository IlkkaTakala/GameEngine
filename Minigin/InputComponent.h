#pragma once
#include "BaseComponent.h"
#include "InputManager.h"

namespace dae {

class InputComponent : public BaseComponent
{
	COMPONENT(InputComponent)

public:

	void SetUserFocus(User user);
	void SetInputEnabled(bool enabled);
	void OnDestroyed() override;

	void BindAction(const std::string& action, std::function<void()> callback);
	void BindAxisAction(const std::string& action, std::function<void(float)> callback);
	void Bind2DAction(const std::string& action, std::function<void(float, float)> callback);

	User GetCurrentUser() const { return CurrentUser; }

	void TriggerAction(int action);
	void TriggerAxisAction(int action, float value);
	void Trigger2DAction(int action, float x, float y);

private:

	User CurrentUser{ 0 };
	bool ReceivesInput{ true };

	std::map<int, std::function<void()>> Actions;
	std::map<int, std::function<void(float)>> ActionsAxis;
	std::map<int, std::function<void(float, float)>> Actions2D;

};

}
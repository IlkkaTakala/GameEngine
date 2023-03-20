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

	void BindAction(int action, std::function<void(void)> callback);
	void BindAxisAction(int action, std::function<void(float)> callback);
	void Bind2DAction(int action, std::function<void(float, float)> callback);

	User GetCurrentUser() const { return CurrentUser; }

	void Trigger2DAction(int action, float x, float y);

private:

	User CurrentUser{ 0 };
	bool ReceivesInput{ true };

	std::map<int, std::function<void(float, float)>> Actions2D;

};

}
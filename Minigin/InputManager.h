#pragma once
#include <map>
#include <functional>
#include <memory>
#include "Singleton.h"

namespace dae
{
	class InputComponent;
	typedef unsigned int User;

	enum class ButtonState
	{
		Down,
		Up,
	};

	class InputManager final : public Singleton<InputManager>
	{
	public:
		InputManager();
		~InputManager();

		bool ProcessInput();

		void RegisterInputComponent(InputComponent* component, User user);
		void UnregisterInputComponent(InputComponent* component);

		ButtonState GetKeyState(User user, unsigned int key);
		float GetAxisValue(User user, unsigned int axis);

	private:
		class InputManagerImpl;
		InputManagerImpl* Impl;
	};

}

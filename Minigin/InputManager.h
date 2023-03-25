#pragma once
#include <map>
#include <functional>
#include <memory>
#include <string>
#include "Singleton.h"
#include "Delegates.h"

namespace dae
{
	class InputComponent;
	typedef unsigned int User;

	enum class ButtonState
	{
		Pressed,
		Up,
	};

	enum class InputMappingType
	{
		Button,
		Axis,
		Axis2D,
		DigitalToY,
		DigitalToNegY,
		DigitalToX,
		DigitalToNegX,
	};

	enum class DeviceType
	{
		Keyboard,
		Mouse,
		Controller
	};

	enum class TriggerType
	{
		Any,
		Pressed,
		Released,
		Down,
		Up,
	};

	struct Key
	{
		int KeyCode;
		DeviceType Type;
		InputMappingType DataType;
		TriggerType Trigger;

		Key(unsigned int code, DeviceType type = DeviceType::Keyboard, InputMappingType dataType = InputMappingType::Button, TriggerType trigger = TriggerType::Pressed)
			: KeyCode(code), Type(type), DataType(dataType), Trigger(trigger) { }
	};

	struct InputMappings
	{
		std::map<int, std::vector<Key>> Keys;
	};

	class InputManager final : public Singleton<InputManager>
	{
	public:
		InputManager();
		~InputManager();

		bool ProcessInput();

		void RegisterInputComponent(InputComponent* component, User user);
		void UnregisterInputComponent(InputComponent* component);

		// TODO: separate controller;
		ButtonState GetKeyState(User user, unsigned int key);
		float GetAxisValue(User user, unsigned int axis);

		void MakeInputMapping(const std::string& name, std::map<std::string, std::vector<Key>>&& data);
		void SetUserMapping(User user, const std::string& name);

		int ActionToActionID(const std::string& action);

		MulticastDelegate<User> OnUserDeviceConnected;
		MulticastDelegate<User> OnUserDeviceDisconnected;

	private:
		class InputManagerImpl;
		InputManagerImpl* Impl;
	};

}

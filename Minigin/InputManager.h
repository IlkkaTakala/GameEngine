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

#pragma region(Constants)
	namespace Buttons
	{
		namespace Controller 
		{
			constexpr unsigned short DpadUp			= 0x0001;
			constexpr unsigned short DpadDown		= 0x0002;
			constexpr unsigned short DpadLeft		= 0x0004;
			constexpr unsigned short DpadRight		= 0x0008;
			constexpr unsigned short Start			= 0x0010;
			constexpr unsigned short Back			= 0x0020;
			constexpr unsigned short LeftThumb		= 0x0040;
			constexpr unsigned short RightThumb		= 0x0080;
			constexpr unsigned short LeftShoulder	= 0x0100;
			constexpr unsigned short RightShoulder	= 0x0200;
			constexpr unsigned short ButtonA		= 0x1000;
			constexpr unsigned short ButtonB		= 0x2000;
			constexpr unsigned short ButtonX		= 0x4000;
			constexpr unsigned short ButtonY		= 0x8000;
		}
		namespace Axis
		{
			constexpr unsigned short Mouse					= 0x0000;
			constexpr unsigned short ControllerStickLeft	= 0x0000;
			constexpr unsigned short ControllerStickRight	= 0x0001;
		}
		namespace Keyboard
		{
			// TODO: Keyboard codes here
		}
	};
#pragma endregion
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

		std::vector<User> GetActiveUsers() const;

		MulticastDelegate<User> OnUserDeviceConnected;
		MulticastDelegate<User> OnUserDeviceDisconnected;

	private:
		class InputManagerImpl;
		InputManagerImpl* Impl;
	};

}

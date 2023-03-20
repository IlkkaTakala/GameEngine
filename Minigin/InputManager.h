#pragma once
#include <map>
#include <functional>
#define	WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#include "Singleton.h"

namespace dae
{
	struct UserData
	{
		XINPUT_STATE State;
		WORD ButtonsReleased;
		WORD ButtonsPressed;

		std::map<int, std::function<void(bool)>> ButtonCommands;
		std::map<int, std::function<void(float)>> AxisCommands;
		std::map<int, std::function<void(float, float)>> StickCommands;
	};

	class InputManager final : public Singleton<InputManager>
	{
	public:
		bool ProcessInput();

	private:

		std::map<int, UserData> Users;
	};

}

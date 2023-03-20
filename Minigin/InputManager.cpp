#include <SDL.h>
#define	WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#include "InputManager.h"
#include "backends/imgui_impl_sdl2.h"
#include "InputComponent.h"

using namespace dae;

struct UserData
{
	XINPUT_STATE State;
	WORD ButtonsReleased;
	WORD ButtonsPressed;
	float LX;
	float LY;
	float RX;
	float RY;

	float LT;
	float RT;

	std::list<InputComponent*> InputStack;
};

class InputManager::InputManagerImpl
{
public:

	bool ProcessInput() {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				return false;
			}
			if (e.type == SDL_KEYDOWN) {
				

			}
			ImGui_ImplSDL2_ProcessEvent(&e);
		}

		for (auto& [user, state] : Users) {

			XINPUT_STATE previousState;

			CopyMemory(&previousState, &state.State, sizeof(XINPUT_STATE));
			ZeroMemory(&state.State, sizeof(XINPUT_STATE));
			XInputGetState(user, &state.State);

			auto buttonChanges = state.State.Gamepad.wButtons ^ previousState.Gamepad.wButtons;
			state.ButtonsPressed = buttonChanges & state.State.Gamepad.wButtons;
			state.ButtonsReleased = buttonChanges & (~state.State.Gamepad.wButtons);

			float LX = state.State.Gamepad.sThumbLX;
			float LY = state.State.Gamepad.sThumbLY;
			float magnitude = sqrt(LX * LX + LY * LY);

			float normalizedLX = LX / 32767;
			float normalizedLY = LY / 32767;

			float normalizedMagnitude = 0;

			if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				if (magnitude > 32767) magnitude = 32767;

				magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

				normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			}
			else
			{
				magnitude = 0.0;
				normalizedMagnitude = 0.0;
			}

			for (auto& in : state.InputStack) {
				in->Trigger2DAction(1, normalizedLX, normalizedLY);
			}
		}

		return true;
	}

	ButtonState GetKeyState(User user, unsigned int key) {
		return Users[user].ButtonsPressed & key ? ButtonState::Down : ButtonState::Up;
	}

	void RegisterInputComponent(InputComponent* component, User user) {
		Users[user].InputStack.push_back(component);
	}

	void UnregisterInputComponent(InputComponent* component) {
		Users[component->GetCurrentUser()].InputStack.remove(component);
	}

	std::map<User, UserData> Users;
};

InputManager::InputManager()
{
	Impl = new InputManagerImpl();
}

dae::InputManager::~InputManager()
{
	delete Impl;
}

bool InputManager::ProcessInput()
{
	return Impl->ProcessInput();
}

void dae::InputManager::RegisterInputComponent(InputComponent* component, User user)
{
	Impl->RegisterInputComponent(component, user);
}

void dae::InputManager::UnregisterInputComponent(InputComponent* component)
{
	Impl->UnregisterInputComponent(component);
}

ButtonState InputManager::GetKeyState(User user, unsigned int key)
{
	return Impl->GetKeyState(user, key);
}

float InputManager::GetAxisValue(User /*user*/, unsigned int /*axis*/)
{
	return 0.0f;
}

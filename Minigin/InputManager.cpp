#include <SDL.h>
#define	WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#include "InputManager.h"
#include "backends/imgui_impl_sdl2.h"
#include "InputComponent.h"
#include "Time.h"

using namespace dae;

struct UserDeviceData
{
	struct Controller {
		XINPUT_STATE State;
		WORD ButtonsReleased;
		WORD ButtonsPressed;
		float LX;
		float LY;
		float RX;
		float RY;

		float LT;
		float RT;

		bool connected;
	} Controller;
	
	InputMappings* Map;
	std::list<ComponentRef<InputComponent>> InputStack;

	UserDeviceData() {
		ZeroMemory(&Controller.State, sizeof(XINPUT_STATE));
		Controller.ButtonsReleased = 0;
		Controller.ButtonsPressed = 0;
		Controller.LX = 0.f;
		Controller.LY = 0.f;
		Controller.RX = 0.f;
		Controller.RY = 0.f;
		Controller.LT = 0.f;
		Controller.RT = 0.f;

		Controller.connected = true;
	}
};

void NormalizeStick(float& x, float& y) 
{
	float magnitude = sqrt(x * x + y * y);

	float normalizedLX = x / 32767;
	float normalizedLY = y / 32767;

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

	x = normalizedLX * normalizedMagnitude;
	y = normalizedLY * normalizedMagnitude;
}

class InputManager::InputManagerImpl
{
public:
	Timer ControllerCheckTimer;
	float ControllerPollInterval{ 1.f };
	InputManager* manager;
	const Uint8* Keyboard;

	InputManagerImpl(InputManager* owner) {
		manager = owner;
		ControllerCheckTimer = Time::GetInstance().SetTimerByEvent(ControllerPollInterval, [this]() { CheckController(); }, true);
		Keyboard = SDL_GetKeyboardState(NULL);
	}

	User XInputUserToPlayer(DWORD user) {
		return User(user + 1);
	}
	DWORD PlayerToXInput(User user) {
		return User(user - 1);
	}

	void CheckController() {
		DWORD dwResult;
		for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));

			dwResult = XInputGetState(i, &state);

			User id = XInputUserToPlayer(i);
			if (dwResult == ERROR_SUCCESS)
			{
				if (auto it = Users.find(id); it == Users.end()) {
					Users.emplace(id, UserDeviceData{});
					manager->OnUserDeviceConnected.Broadcast(id);
				}
				else {
					if (!it->second.Controller.connected) manager->OnUserDeviceConnected.Broadcast(id);
					it->second.Controller.connected = true;
				}
			}
			else {
				if (auto it = Users.find(id); it != Users.end()) {
					if (it->second.Controller.connected) manager->OnUserDeviceDisconnected.Broadcast(id);
					it->second.Controller.connected = false;
				}
			}
		}
	}

	bool ProcessInput() {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				return false;
			}
			if (e.type == SDL_KEYDOWN) {
				if (Users[0].Map) {
					for (auto& [action, keys] : Users[0].Map->Keys) {
						for (auto& key : keys) {
							if (key.Trigger == TriggerType::Pressed && key.Type == DeviceType::Keyboard) {
								if (key.KeyCode == e.key.keysym.sym) {
									for (auto& in : Users[0].InputStack) {
										in->TriggerAction(action);
									}
								}
							}
						}
					}
				}
			}
			if (e.type == SDL_KEYUP) {
				if (Users[0].Map) {
					for (auto& [action, keys] : Users[0].Map->Keys) {
						for (auto& key : keys) {
							if (key.Trigger == TriggerType::Up && key.Type == DeviceType::Keyboard) {
								if (key.KeyCode == e.key.keysym.sym) {
									for (auto& in : Users[0].InputStack) {
										in->TriggerAction(action);
									}
								}
							}
						}
					}
				}
			}
			ImGui_ImplSDL2_ProcessEvent(&e);
		}

		if (Users[0].Map) {
			for (auto& [action, keys] : Users[0].Map->Keys) {
				float x = 0.f, y = 0.f;
				bool hasX = false, hasY = false;
				for (auto& key : keys) {
					if (key.Type == DeviceType::Keyboard) {
						auto scan = SDL_GetScancodeFromKey(key.KeyCode);
						switch (key.DataType)
						{
						case InputMappingType::Button: {
							if ((key.Trigger == TriggerType::Down && Keyboard[scan])
								|| (key.Trigger == TriggerType::Up && !Keyboard[scan])) 
							{
								for (auto& in : Users[0].InputStack) {
									in->TriggerAction(action);
								}
							}
						} break;

						case InputMappingType::DigitalToY: {
							hasY = true;
							if (Keyboard[scan]) {
								y += 1.f;
							}
						} break;

						case InputMappingType::DigitalToNegY: {
							hasY = true;
							if (Keyboard[scan]) {
								y += -1.f;
							}
						} break;

						case InputMappingType::DigitalToX: {
							hasX = true;
							if (Keyboard[scan]) {
								x += 1.f;
							}
						} break;

						case InputMappingType::DigitalToNegX: {
							hasX = true;
							if (Keyboard[scan]) {
								x += -1.f;
							}
						} break;
						}
					}
				}
				float mag = sqrt(x * x + y * y);
				if (mag > 1.f) {
					x /= mag;
					y /= mag;
				}
				for (auto& in : Users[0].InputStack) {
					if (hasX && hasY) {
						in->Trigger2DAction(action, x, y);
					}
					else if (hasX || hasY) {
						float value = abs(x) < abs(y) ? y : x;
						in->TriggerAxisAction(action, value);
					}
				}
			}
		}

		for (auto& [user, state] : Users) {

			if (!state.Controller.connected) continue;
			XINPUT_STATE previousState;

			CopyMemory(&previousState, &state.Controller.State, sizeof(XINPUT_STATE));
			ZeroMemory(&state.Controller.State, sizeof(XINPUT_STATE));
			XInputGetState(PlayerToXInput(user), &state.Controller.State);

			auto buttonChanges = state.Controller.State.Gamepad.wButtons ^ previousState.Gamepad.wButtons;
			state.Controller.ButtonsPressed = buttonChanges & state.Controller.State.Gamepad.wButtons;
			state.Controller.ButtonsReleased = buttonChanges & (~state.Controller.State.Gamepad.wButtons);

			float LX = state.Controller.State.Gamepad.sThumbLX;
			float LY = state.Controller.State.Gamepad.sThumbLY;
			float RX = state.Controller.State.Gamepad.sThumbRX;
			float RY = state.Controller.State.Gamepad.sThumbRY;
			NormalizeStick(LX, LY);
			NormalizeStick(RX, RY);
			state.Controller.LX = LX;
			state.Controller.LY = LY;
			state.Controller.RX = RX;
			state.Controller.RY = RY;
			state.Controller.LT = (float)state.Controller.State.Gamepad.bLeftTrigger / 255.f;
			state.Controller.RT = (float)state.Controller.State.Gamepad.bRightTrigger / 255.f;


			if (!state.Map) continue;
			for (auto& [action, keys] : state.Map->Keys) {
				float x = 0.f, y = 0.f;
				bool hasX = false, hasY = false;
				for (auto& k : keys) {
					if (k.Type == DeviceType::Controller) {
						switch (k.DataType)
						{
						case InputMappingType::Button: {
							bool down = Users[user].Controller.State.Gamepad.wButtons & k.KeyCode;
							bool pressed = Users[user].Controller.ButtonsPressed & k.KeyCode;
							bool released = Users[user].Controller.ButtonsPressed & k.KeyCode;
							if (k.Trigger == TriggerType::Any 
								|| (pressed && k.Trigger == TriggerType::Pressed) 
								|| (released && k.Trigger == TriggerType::Released)
								|| (down && k.Trigger == TriggerType::Down)
								|| (!down && k.Trigger == TriggerType::Up)) 
							{
								for (auto& in : state.InputStack) {
									in->TriggerAction(action);
								}
							}
						} break;

						case InputMappingType::Axis: {
							switch (k.KeyCode)
							{
							case 0: {
								hasY = true;
								y += state.Controller.LT;
							} break;

							case 1: {
								hasY = true;
								y += state.Controller.RT;
							} break;
							}
						} break;

						case InputMappingType::Axis2D: {
							switch (k.KeyCode)
							{
							case 0: {
								hasX = true;
								hasY = true;
								x += state.Controller.LX;
								y += state.Controller.LY;
							} break;

							case 1: {
								hasX = true;
								hasY = true;
								x += state.Controller.RX;
								y += state.Controller.RY;
							} break;
							}
						} break;

						case InputMappingType::DigitalToY: {
							hasY = true;
							if (state.Controller.State.Gamepad.wButtons & k.KeyCode) {
								y += 1.f;
							}
						} break;

						case InputMappingType::DigitalToNegY: {
							hasY = true;
							if (state.Controller.State.Gamepad.wButtons & k.KeyCode) {
								y += -1.f;
							}
						} break;

						case InputMappingType::DigitalToX: {
							hasX = true;
							if (state.Controller.State.Gamepad.wButtons & k.KeyCode) {
								x += 1.f;
							}
						} break;

						case InputMappingType::DigitalToNegX: {
							hasX = true;
							if (state.Controller.State.Gamepad.wButtons & k.KeyCode) {
								x += -1.f;
							}
						} break;
						default:
							break;
						}
					}
				}

				float mag = sqrt(x * x + y * y);
				if (mag > 1.f) {
					x /= mag;
					y /= mag;
				}
				for (auto& in : state.InputStack) {
					if (hasX && hasY) {
						in->Trigger2DAction(action, x, y);
					}
					else if (hasX || hasY) {
						float value = abs(x) < abs(y) ? y : x;
						in->TriggerAxisAction(action, value);
					}
				}
			}
		}

		return true;
	}

	ButtonState GetKeyState(User user, unsigned int key) {
		return Users[user].Controller.ButtonsPressed & key ? ButtonState::Pressed : ButtonState::Up;
	}

	void RegisterInputComponent(InputComponent* component, User user) {
		Users[user].InputStack.remove(component);
		Users[user].InputStack.push_back(component->GetPermanentReference());
	}

	void UnregisterInputComponent(InputComponent* component) {
		Users[component->GetCurrentUser()].InputStack.remove(component->GetPermanentReference());
	}

	void MakeInputMapping(const std::string& name, std::map<std::string, std::vector<Key>>&& data) {
		InputMappings map;
		int count = (int)ActionToID.size();
		for (auto& [action, keys] : data) {
			int id = 0;
			if (auto it = ActionToID.find(action); it != ActionToID.end()) {
				id = it->second;
			}
			else {
				ActionToID[action] = id = ++count;
			}
			map.Keys.emplace(id, keys);
		}
		AllinputMappings.emplace(name, std::move(map));
	}

	void SetUserMapping(User user, const std::string& name) {
		if (auto it = AllinputMappings.find(name); it != AllinputMappings.end()) {
			Users[user].Map = &it->second;
		}
	}

	int ActionToActionID(const std::string& action) {
		if (auto it = ActionToID.find(action); it != ActionToID.end()) {
			return it->second;
		}
		return -1;
	}

	std::map<std::string, int> ActionToID;
	std::map<std::string, InputMappings> AllinputMappings;
	std::map<User, UserDeviceData> Users;
};

InputManager::InputManager()
{
	Impl = new InputManagerImpl(this);
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

void dae::InputManager::MakeInputMapping(const std::string& name, std::map<std::string, std::vector<Key>>&& data)
{
	Impl->MakeInputMapping(name, std::move(data));
}

void dae::InputManager::SetUserMapping(User user, const std::string& name)
{
	Impl->SetUserMapping(user, name);
}

int dae::InputManager::ActionToActionID(const std::string& action)
{
	return Impl->ActionToActionID(action);
}

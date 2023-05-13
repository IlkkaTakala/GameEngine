#include "SystemManager.h"

SoundManager* SystemManager::soundsystem = nullptr;

void SystemManager::RegisterSoundSystem(SoundManager* system)
{
	if (soundsystem) delete soundsystem;
	soundsystem = system;
}

SoundManager* SystemManager::GetSoundSystem()
{
	return soundsystem;
}

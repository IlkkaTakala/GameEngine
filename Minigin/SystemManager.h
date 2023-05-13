#pragma once
#include "SoundManager.h"

class SystemManager final
{
public:

	static void RegisterSoundSystem(SoundManager* system);
	static SoundManager* GetSoundSystem();

private:

	static SoundManager* soundsystem;
};


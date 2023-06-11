#include "GameGlobals.h"
#include "SystemManager.h"

void GameGlobals::ToggleMuted()
{
	using namespace dae;
	Muted = !Muted;

	SystemManager::GetSoundSystem()->MuteSounds(Muted);
}

#pragma once
#include "SoundManager.h"
#include <map>

class SDL_SoundSystem final : public SoundManager
{
public:

	SDL_SoundSystem();
	~SDL_SoundSystem();

	void SetDataPath(const std::string& path) override;
	void Play(const Sound& sound, const PlayParameters& params = {}) override;
	void SetSoundParameters(const Sound& sound, const SoundParameters& params) override;
	Sound GetSound(const std::string& name) override;
	void SetVolume(Sound sound, float volume) override;
	void StopSound(Sound sound) override;
	void ReleaseSound(Sound sound) override;
	void MuteSounds(bool muted) override;

private:
	 
	class SoundImpl;
	SoundImpl* Impl;
};


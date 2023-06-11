#pragma once
#include <string>

using Sound = size_t;

struct SoundParameters
{
	bool AutoRelease{ true };
	float Volume{ 1.f };
};

struct PlayParameters
{
	float Volume{ 1.f };
};

class SoundManager
{
public:
	SoundManager() {}

	void Play(const std::string& sound, const PlayParameters& params = {}) { Play(GetSound(sound), params); };

	virtual void SetDataPath(const std::string& path) = 0;
	virtual void Play(const Sound& sound, const PlayParameters& params = {}) = 0;
	virtual void SetSoundParameters(const Sound& sound, const SoundParameters& params) = 0;
	virtual Sound GetSound(const std::string& name) = 0;
	virtual void SetVolume(Sound sound, float volume) = 0;
	virtual void StopSound(Sound sound) = 0;
	virtual void ReleaseSound(Sound sound) = 0;
	virtual void MuteSounds(bool muted) = 0;

	virtual ~SoundManager() = default;
	SoundManager(const SoundManager& other) = delete;
	SoundManager(SoundManager&& other) = delete;
	SoundManager& operator=(const SoundManager& other) = delete;
	SoundManager& operator=(SoundManager&& other) = delete;

protected:
};
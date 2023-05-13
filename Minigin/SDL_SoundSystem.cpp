#include "SDL_SoundSystem.h"
#include <thread>
#include <condition_variable>
#include <queue>
#include "SDL_mixer.h"
#include "SDL.h"

template <class T>
class SafeQueue
{
public:
	SafeQueue(void)
		: Queue()
		, m()
		, c()
	{}

	~SafeQueue(void)
	{}

	void push(T t)
	{
		std::lock_guard<std::mutex> lock(m);
		Queue.push(t);
		c.notify_one();
	}

	T pop(void)
	{
		std::unique_lock<std::mutex> lock(m);
		c.wait(lock, [&] { return !Queue.empty(); });
		T val = Queue.front();
		Queue.pop();
		return val;
	}

	void clear() 
	{
		std::lock_guard<std::mutex> lock(m);
		while (!Queue.empty()) {
			Queue.pop();
		}
	}

private:
	std::queue<T> Queue;
	mutable std::mutex m;
	std::condition_variable c;
};

enum class SoundEvent
{
	Init,
	Load,
	Play,
	Stop,
	Volume,
	Release,
	Quit,
};

void SoundFinished(int) {

}

class SDL_SoundSystem::SoundImpl
{
public:
	SoundImpl() {
		
		Running = true;
		Thread = new std::thread(&SDL_SoundSystem::SoundImpl::ThreadedHandler, this);
		EventQueue.push({ SoundEvent::Init, 0 });
	}

	~SoundImpl() {
		EventQueue.clear();
		EventQueue.push({ SoundEvent::Quit, 0 });
		Thread->join();
		delete Thread;
	}

	void SetDataPath(const std::string& path) {
		DataPath = path;
	}

	void Play(const Sound& sound, const PlayParameters& params) {
		std::lock_guard<std::mutex> lock(QueueMutex);
		Queue.push({ sound, params });
		EventQueue.push({ SoundEvent::Play, sound });
	}
	
	void SetSoundParameters(const Sound& sound, const SoundParameters& params) {
		LoadedSounds[sound].second.Params = params;
	}

	Sound GetSound(const std::string& name) {
		if (auto it = NameTable.find(name); it != NameTable.end()) {
			return it->second;
		}

		Sound sound;
		if (!FreeList.empty()) {
			sound = FreeList.front();
			FreeList.pop();

			LoadedSounds[sound].second.Name = name;
		}
		else {
			sound = LoadedSounds.size();
			LoadedSounds.push_back({ nullptr, { -1, false, false, name, {}} });
		}

		NameTable.emplace(name, sound);
		EventQueue.push({ SoundEvent::Load, sound });

		return sound;
	}

	void SetVolume(Sound sound, float volume) {
		LoadedSounds[sound].second.Params.Volume = volume;
		EventQueue.push({ SoundEvent::Volume, sound });
	}

	void StopSound(Sound sound) {
		EventQueue.push({ SoundEvent::Stop, sound });
	}

	void ReleaseSound(Sound sound) {
		EventQueue.push({ SoundEvent::Release, sound });
	}

private:

	struct InternalParams
	{
		int channel{ -1 };
		bool Loaded{ false };
		bool Playing{ false };
		std::string Name;

		SoundParameters Params;
	};

	int baseVolume{ 32 };

	void ThreadedHandler()
	{
		while (Running) {
			auto [Event, Sound] = EventQueue.pop();
			switch (Event)
			{
				case SoundEvent::Init: {
					SDL_Init(SDL_INIT_AUDIO);
					Mix_Init(MIX_INIT_MP3);
					Mix_OpenAudio(48000, AUDIO_F32SYS, 2, 2048);

					Mix_ChannelFinished(SoundFinished);
				} break;
				case SoundEvent::Load: {
					auto chunk = Mix_LoadWAV((DataPath + LoadedSounds[Sound].second.Name).c_str());
					Mix_VolumeChunk(chunk, int(LoadedSounds[Sound].second.Params.Volume * baseVolume));
					LoadedSounds[Sound].first = chunk;
					LoadedSounds[Sound].second.Loaded = chunk != nullptr;
				} break;
				case SoundEvent::Play: {
					std::lock_guard<std::mutex> lock(QueueMutex);
					auto [play, data] = Queue.front();
					if (LoadedSounds[play].second.Loaded) {
						Queue.pop();
						int channel = Mix_PlayChannel(-1, LoadedSounds[play].first, 0);
						Mix_Volume(channel, int(data.Volume * baseVolume));
						LoadedSounds[play].second.channel = channel;
						LoadedSounds[play].second.Playing = channel != -1;
					} else EventQueue.push({ SoundEvent::Play, 0 });
				} break;
				case SoundEvent::Stop: {
					if (LoadedSounds[Sound].second.Playing)
						Mix_FadeOutChannel(LoadedSounds[Sound].second.channel, 300);
				} break;
				case SoundEvent::Volume: {
					if (LoadedSounds[Sound].second.Playing)
						Mix_VolumeChunk(LoadedSounds[Sound].first, int(LoadedSounds[Sound].second.Params.Volume * baseVolume));
				} break;
				case SoundEvent::Release: {
					Mix_FreeChunk(LoadedSounds[Sound].first);
					LoadedSounds[Sound].second.channel = -1;
					LoadedSounds[Sound].second.Loaded = false;
					LoadedSounds[Sound].second.Playing = false;
					LoadedSounds[Sound].second.Params = {};
					LoadedSounds[Sound].second.Name = "";
					LoadedSounds[Sound].first = nullptr;
					FreeList.push(Sound);
				} break;
				case SoundEvent::Quit: {
					for (auto& [chunk, data] : LoadedSounds) {
						Mix_FreeChunk(chunk);
					}
					LoadedSounds.clear();
					NameTable.clear();
					Running = false;
					Mix_Quit();
				} break;
			default:
				break;
			}
		}
	}

	bool Running;

	std::thread* Thread{ nullptr };
	std::string DataPath;
	std::queue<std::pair<Sound, PlayParameters>> Queue;
	std::mutex QueueMutex;
	SafeQueue<std::pair<SoundEvent, Sound>> EventQueue;

	std::queue<size_t> FreeList;
	std::vector<std::pair<Mix_Chunk*, InternalParams>> LoadedSounds;
	std::map<std::string, Sound> NameTable;
};

SDL_SoundSystem::SDL_SoundSystem()
{
	Impl = new SoundImpl();
}

SDL_SoundSystem::~SDL_SoundSystem()
{
	delete Impl;
}

void SDL_SoundSystem::SetDataPath(const std::string& path)
{
	Impl->SetDataPath(path);
}

void SDL_SoundSystem::Play(const Sound& sound, const PlayParameters& params)
{
	Impl->Play(sound, params);
}

void SDL_SoundSystem::SetSoundParameters(const Sound& sound, const SoundParameters& params)
{
	Impl->SetSoundParameters(sound, params);
}

Sound SDL_SoundSystem::GetSound(const std::string& name)
{
	return Impl->GetSound(name);
}

void SDL_SoundSystem::SetVolume(Sound sound, float volume)
{
	Impl->SetVolume(sound, volume);
}

void SDL_SoundSystem::StopSound(Sound sound)
{
	Impl->StopSound(sound);
}

void SDL_SoundSystem::ReleaseSound(Sound sound)
{
	Impl->ReleaseSound(sound);
}

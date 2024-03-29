#include "EngineTime.h"
#include <thread>
#include <future>

void dae::Time::Update(float delta)
{
	LastDelta = delta;
	TotalTime += delta;

	std::erase_if(Timers,
		[this](const std::pair<size_t, TimerData>& item) -> bool {
			return std::find(RemoveList.begin(), RemoveList.end(), item.first) != RemoveList.end();
		});
	for (auto& h : RemoveList) FreeList.push(h);
	RemoveList.clear();
	Timers.insert(NewTimers.begin(), NewTimers.end());
	NewTimers.clear();
	for (auto& [handle, t] : Timers) {
		t.elapsed += delta;
		if (t.elapsed >= t.duration) {
			t.timer();
			if (t.looping) {
				t.elapsed -= t.duration;
			}
			else {
				ClearTimer(handle);
			}
		}
	}

	for (auto& [h, t] : Asyncs) {
		if (t && t->cleared) {
			delete t;
			t = nullptr;
		}
	}
	std::erase_if(Asyncs, [](auto& t) { return t.second == nullptr; });
}

dae::Timer dae::Time::SetTimerByEvent(float time, const TimerCallback& callback, bool looping)
{
	size_t id{ 0 };
	if (!FreeList.empty()) {
		id = FreeList.front();
		FreeList.pop();
	}
	else {
		id = ++Last;
	}

	TimerData t{
		callback,
		time,
		0.f,
		looping
	};

	NewTimers.emplace(id, std::move(t));

	return id;
}

dae::AsyncTimer dae::Time::LaunchAsyncTimerByEvent(float time, TimerCallback callback, bool looping)
{
	auto t = new AsyncTimerData();
	size_t id = ++LastAsync;
	Asyncs.emplace(id, t);
	t->future = std::async(std::launch::async, &Time::AsyncFunc, this, t, callback, time, looping);
	return id;
}

float dae::Time::GetRemainingTime(const Timer& handle)
{
	if (auto it = Timers.find(handle); it != Timers.end()) {
		return it->second.duration - it->second.elapsed;
	}
	return 0.0f;
}

float dae::Time::GetElapsedTime(const Timer& handle)
{
	if (auto it = Timers.find(handle); it != Timers.end()) {
		return it->second.elapsed;
	}
	return 0.0f;
}

void dae::Time::ClearTimer(const Timer& handle)
{
	if (handle == 0) return;

	if (std::find(RemoveList.begin(), RemoveList.end(), handle) == RemoveList.end())
		RemoveList.push_back(handle);
}

void dae::Time::ClearAsyncTimer(const AsyncTimer& handle)
{
	if (auto it = Asyncs.find(handle); it != Asyncs.end())
		it->second->close = true;
}

void dae::Time::ClearAllTimers()
{
	Timers.clear();
	RemoveList.clear();
	Last = 0;
	std::queue<size_t> empty;
	std::swap(FreeList, empty);

	for (auto& [h, t] : Asyncs) {
		t->close = true;
		t->future.wait();
		delete t;
	}
	Asyncs.clear();
}

void dae::Time::AsyncFunc(AsyncTimerData* data, const TimerCallback& callback, float duration, bool loop)
{
	while (loop && !data->close) {
		callback();
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(duration * 1000)));
	}
	data->cleared = true;
}

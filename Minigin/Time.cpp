#include "EngineTime.h"
#include <thread>
#include <future>

void dae::Time::Update(float delta)
{
	LastDelta = delta;
	TotalTime += delta;

	std::vector<size_t> RemoveList;
	for (auto& [handle, t] : Timers) {
		t.elapsed += delta;
		if (t.elapsed >= t.duration) {
			t.timer();
			if (t.looping) {
				t.elapsed -= t.duration;
			}
			else {
				RemoveList.push_back(handle);
				FreeList.push(handle);
			}
		}
	}
	std::erase_if(Timers,
		[&RemoveList](const std::pair<size_t, TimerData>& item) -> bool {
			return std::find(RemoveList.begin(), RemoveList.end(), item.first) != RemoveList.end();
		});

	for (auto& t : Asyncs) {
		if (t->cleared) {
			delete t;
			t = nullptr;
		}
	}
	Asyncs.remove_if([](auto t) { return t == nullptr; });
}

dae::Timer dae::Time::SetTimerByEvent(float time, const TimerCallback& callback, bool looping)
{
	size_t id{ 0 };
	if (!FreeList.empty()) {
		id = FreeList.front();
		FreeList.pop();
	}
	else {
		id = Last++;
	}

	TimerData t{
		callback,
		time,
		0.f,
		looping
	};

	Timers.emplace(id, std::move(t));

	return id;
}

dae::AsyncTimer dae::Time::LaunchAsyncTimerByEvent(float time, TimerCallback callback, bool looping)
{
	auto t = new AsyncTimerData();
	Asyncs.push_back(t);
	t->future = std::async(std::launch::async, &Time::AsyncFunc, this, t, callback, time, looping);
	return t;
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
	Timers.erase(handle);
	FreeList.push(handle);
}

void dae::Time::ClearTimer(const AsyncTimer& handle)
{
	((AsyncTimerData*)handle)->close = true;
}

void dae::Time::ClearAllTimers()
{
	Timers.clear();
	std::queue<size_t> empty;
	std::swap(FreeList, empty);

	for (auto& t : Asyncs) {
		t->close = true;
		t->future.get();
		delete t;
	}
}

void dae::Time::AsyncFunc(AsyncTimerData* data, const TimerCallback& callback, float duration, bool loop)
{
	while (loop && !data->close) {
		callback();
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(duration * 1000)));
	}
	data->cleared = true;
}

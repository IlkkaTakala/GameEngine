#pragma once
#include "Singleton.h"
#include <functional>
#include <map>
#include <queue>

namespace dae 
{

	typedef size_t Timer;
	typedef std::function<void(void)> TimerCallback;

	struct TimerData
	{
		TimerCallback timer;
		float duration{ 0.f };
		float elapsed{ 0.f };
		bool looping{ false };
	};

	class Time final : public Singleton<Time>
	{
	public: 

		void Update(float delta);

		Timer SetTimerByEvent(float time, const TimerCallback& callback, bool looping = false);
		float GetRemainingTime(const Timer& handle);
		float GetElapsedTime(const Timer& handle);

		void ClearTimer(const Timer& handle);
		void ClearAllTimers();

		float GetDelta() { return LastDelta; }
		float GetTotal() { return TotalTime; }

	private:

		float LastDelta{ 0.f };
		float TotalTime{ 0.f };

		size_t Last{ 0 };
		std::map<size_t, TimerData> Timers;
		std::queue<size_t> FreeList;
	};

}

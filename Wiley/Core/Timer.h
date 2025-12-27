#pragma once

#include <string>
#include <string_view>
#include <ctime>
#include <chrono>

namespace Wiley {

#define WILEY_START_TIMER CPUTimer::timer().start()
#define WILEY_STOP_TIMER  CPUTimer::timer().stop()
#define WILEY_RESET_TIMER CPUTimer::timer().reset()
#define WILEY_GET_LAST_TIMER_DURATION(x) (x) = CPUTimer::timer().durationMs()
#define WILEY_SYS_TIME(t) CPUTimer::timer().getSystemTime(t)

	class ITimer
	{
	public:
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void reset() = 0;

		virtual long long durationMs() = 0;

		virtual void getSystemTime(std::string& t) = 0;
		~ITimer() = default;
	};

	class CPUTimer : public ITimer
	{
	public:
		using HRClock = std::chrono::high_resolution_clock;
		using SysClock = std::chrono::system_clock;
		using ClockMS = std::chrono::milliseconds;

		static CPUTimer& timer();

		virtual void start();
		virtual void stop();
		virtual void reset();

		virtual long long durationMs();
		virtual void getSystemTime(std::string& t);

		HRClock::time_point mStart;
		HRClock::time_point mEnd;
		HRClock::duration mDuration;
	};
}
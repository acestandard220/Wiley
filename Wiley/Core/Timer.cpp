#include "timer.h"

namespace Wiley {

    CPUTimer timer;

    CPUTimer& CPUTimer::timer()
    {
        return Wiley::timer;
    }

    void CPUTimer::start()
    {
        mStart = HRClock::now();
    }

    void CPUTimer::stop()
    {
        mEnd = HRClock::now();
        mDuration = std::chrono::duration_cast<ClockMS>(mEnd - mStart);
    }

    void CPUTimer::reset()
    {
        mStart = HRClock::now();
        mDuration = ClockMS::zero();
    }

    long long CPUTimer::durationMs()
    {
        return std::chrono::duration<double, std::milli>(mEnd - mStart).count();
    }

    void CPUTimer::getSystemTime(std::string& t)
    {
        // Get current time as time_point
        auto now = std::chrono::system_clock::now();

        // Convert to time_t (C-style time)
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        // Convert to local time
        std::tm local_tm;
#ifdef _WIN32
        localtime_s(&local_tm, &now_time);  // thread-safe on Windows
#else
        localtime_r(&now_time, &local_tm);  // thread-safe on Linux/macOS
#endif

        // Format as hh:mm AM/PM
        std::ostringstream oss;
        oss << std::put_time(&local_tm, "%I:%M %p"); // 12-hour format with AM/PM
        t = oss.str();
    }
}
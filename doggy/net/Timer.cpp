#include "doggy/net/Timer.h"

using namespace doggy;
using namespace doggy::net;

using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;

std::atomic<int64_t> Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
        if (repeat_)
        {
                *expiration_ = now + interval_;
        }
        else
        {
                expiration_ = std::nullopt;
        }
}
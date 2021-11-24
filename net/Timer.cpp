#pragma once

#include "./Timer.h"

using namespace doggy;
using namespace doggy::base;

std::atomic<int64_t> Timer::s_numCreated_;

void Timer::restart(std::chrono::microseconds now)
{
        if (repeat_)
        {
                expiration_ = now + interval_;
        }
        else
        {
                expiration_ = 0us;
        }
}
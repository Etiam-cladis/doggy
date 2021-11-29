#pragma once

#include "./Timer.h"

namespace doggy
{
        namespace net
        {
                class TimerId
                {
                public:
                        TimerId()
                            : timer_(NULL),
                              sequence_(0)
                        {
                        }

                        TimerId(Timer *timer, int64_t seq)
                            : timer_(timer),
                              sequence_(seq)
                        {
                        }

                        bool hasTimer() const { return timer_ != nullptr; };

                        friend class TimerQueue;

                private:
                        Timer *timer_;
                        int64_t sequence_;
                };

        } // namespace net
} // namespace  doggy

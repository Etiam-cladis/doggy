#ifndef DOGGY_NET_TIMERID_H
#define DOGGY_NET_TIMERID_H

#include "doggy/net/Timer.h"

namespace doggy
{
        namespace net
        {
                class Timer;

                class TimerId
                {
                public:
                        TimerId()
                            : timer_(nullptr),
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

#endif // DOGGY_NET_TIMERID_H

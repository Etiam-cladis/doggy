#pragma once

#include "./header.h"

#include "../net/Channel.h"
#include "../net/EventLoop.h"

namespace doggy
{
        namespace net
        {
                using namespace std::chrono_literals;
                class Timer
                {
                        using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
                        using TimerCallback = std::function<void()>;

                public:
                        Timer(const TimerCallback &cb, Timestamp when, std::chrono::microseconds interval)
                            : callback_(cb),
                              interval_(interval),
                              repeat_(interval > 0us),
                              sequence_(s_numCreated_.fetch_add(1) + 1)
                        {
                        }

                        Timer(TimerCallback &&cb, Timestamp when, std::chrono::microseconds interval)
                            : callback_(std::move(cb)),
                              interval_(interval),
                              repeat_(interval > 0us),
                              sequence_(s_numCreated_.fetch_add(1) + 1)
                        {
                        }

                        void run() const
                        {
                                callback_();
                        }

                        Timestamp expiration() const { return expiration_.value_or(Timestamp()); }
                        bool repeat() const { return repeat_; }
                        int64_t sequence() const { return sequence_; }

                        void restart(Timestamp now);

                private:
                        const TimerCallback callback_;
                        std::optional<Timestamp> expiration_;
                        const std::chrono::microseconds interval_;
                        const bool repeat_;
                        const int64_t sequence_;

                        static std::atomic<int64_t> s_numCreated_;
                };

        } // namespace net

} // namespace doggy

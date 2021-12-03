#ifndef DOGGY_NET_EVENTLOOP_H
#define DOGGY_NET_EVENTLOOP_H

#include "doggy/net/header.h"

#include "doggy/net/Timer.h"
#include "doggy/net/TimerId.h"

namespace doggy
{
        namespace net
        {
                class Channel;
                class Epoll;
                class TimerQueue;

                class EventLoop
                {
                        using Functor = std::function<void()>;
                        using ChannelList = std::vector<Channel *>;
                        using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
                        using TimerCallback = std::function<void()>;

                public:
                        EventLoop();
                        ~EventLoop();

                public:
                        EventLoop(const EventLoop &) = delete;
                        EventLoop &operator=(const EventLoop &) = delete;

                public:
                        void loop();
                        void quit();
                        void wakeup();

                        void runInLoop(const Functor &cb);
                        void runInLoop(Functor &&cb);
                        void queueInLoop(const Functor &cb);
                        void queueInLoop(Functor &&cb);
                        size_t queueSize() const;

                        TimerId runAt(Timestamp time, const TimerCallback &cb);
                        TimerId runAt(Timestamp time, TimerCallback &&cb);
                        TimerId runAfter(std::chrono::microseconds delay, const TimerCallback &cb);
                        TimerId runAfter(std::chrono::microseconds delay, TimerCallback &&cb);
                        TimerId runEvery(std::chrono::microseconds interval, const TimerCallback &cb);
                        TimerId runEvery(std::chrono::microseconds interval, TimerCallback &&cb);
                        void cancel(TimerId timerId);

                        void updateChannelRW(Channel *channel);
                        void updateChannelOther(Channel *channel);
                        void removeChannel(Channel *channel);

                        void assertInLoopThread()
                        {
                                if (!isInLoopThread())
                                        abortNotInThreadLoop();
                        }
                        bool isInLoopThread() { return threadId_ == std::this_thread::get_id(); }

                private:
                        void abortNotInThreadLoop()
                        {
                                // LOG FATAL TODO
                                abort();
                        }

                        void wakeupReadHanding();
                        void doPendingFunctors();

                private:
                        std::atomic<bool> looping_;
                        std::atomic<bool> quit_;
                        std::atomic<bool> eventHandling_;
                        std::thread::id threadId_;

                        std::unique_ptr<Epoll> poller_;
                        std::unique_ptr<TimerQueue> timerQueue_;

                        int wakeupFd_;
                        std::unique_ptr<Channel> wakeupChannel_;

                        ChannelList activeChannelList_;
                        Channel *currentChannel_;

                        std::atomic<bool> callPending_;
                        mutable std::mutex pendingQueueMutex_;
                        std::vector<Functor> pendingQueue_;
                };

        } // namespace net

} // namespace doggy

#endif // DOGGY_NET_EVENTLOOP_H

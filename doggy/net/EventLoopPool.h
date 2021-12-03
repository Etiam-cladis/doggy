#ifndef DOGGY_NET_EVENTLOOPPOOL_H
#define DOGGY_NET_EVENTLOOPPOOL_H

#include "doggy/net/header.h"

#include "doggy/net/EventLoop.h"

namespace doggy
{
        namespace net
        {
                class EventLoopThread;

                class EventLoopPool
                {
                        using ThreadInitCallback = std::function<void(EventLoop *)>;

                public:
                        EventLoopPool(unsigned int numThreads);
                        ~EventLoopPool();

                public:
                        EventLoopPool(const EventLoopPool &) = delete;
                        EventLoopPool &operator=(const EventLoopPool &) = delete;

                public:
                        void start(const ThreadInitCallback &cb);

                        // vaild call after start()
                        EventLoop *getNextLoop();

                        // Func  should be  like void()
                        // TODO other Func format
                        template <typename Func, typename = std::enable_if_t<std::is_invocable_v<Func>>>
                        void runInPool(Func &&func);

                        // set number of threads before started or do nothing.
                        void setNumThreads(unsigned int num)
                        {
                                if (!isStarted())
                                        numThreads_ = num;
                        };

                        bool isStarted() { return started_.load(std::memory_order_acquire); };

                private:
                        std::atomic<bool> started_;

                        unsigned int numThreads_;
                        std::vector<std::unique_ptr<EventLoopThread>> threads_;

                        int nextLoop_;
                        std::vector<EventLoop *> loops_;
                };

                template <typename Func, typename>
                void EventLoopPool::runInPool(Func &&func)
                {
                        assert(started_.load(std::memory_order_relaxed));
                        EventLoop *lp = getNextLoop();
                        lp->runInLoop(std::forward<Func>(func));
                }

        } // namespace net

} // namespace doggy

#endif // DOGGY_NET_EVENTLOOPPOOL_H

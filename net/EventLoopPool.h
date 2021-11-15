#pragma once

#include "./header.h"

#include "./EventLoop.h"

namespace doggy
{
        namespace net
        {
                class EventLoopPool
                {
                public:
                        EventLoopPool(unsigned int numThreads = std::thread::hardware_concurrency());
                        ~EventLoopPool();

                public:
                        EventLoopPool(const EventLoopPool &) = delete;
                        EventLoopPool &operator=(const EventLoopPool &) = delete;

                public:
                        void start();
                        void stopAll();

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
                        std::vector<std::thread> threads_;

                        int nextLoop_;
                        std::vector<std::unique_ptr<EventLoop>> loops_;
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
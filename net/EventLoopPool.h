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
                        EventLoopPool(EventLoop *baseLoop, unsigned int numThreads = std::thread::hardware_concurrency());
                        ~EventLoopPool();

                public:
                        EventLoopPool(const EventLoopPool &) = delete;
                        EventLoopPool &operator=(const EventLoopPool &) = delete;

                public:
                        void start();

                        // set number of threads before started or do nothing.
                        void setNumThreads(unsigned int num)
                        {
                                if (!isStarted())
                                        numThreads_ = num;
                        };

                        EventLoop *getNextLoop();
                        std::vector<EventLoop *> getAllLoops();

                        bool isStarted() { return started_.load(std::memory_order_acquire); };

                private:
                        EventLoop *baseLoop_;
                        std::atomic<bool> started_;
                        unsigned int numThreads_;
                        std::vector<std::thread> threads_;

                        int nextLoop_;
                        std::vector<EventLoop *> loops_;
                };

        } // namespace net

} // namespace doggy
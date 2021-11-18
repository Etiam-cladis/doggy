#pragma once

#include "../base/header.h"

#include "./EventLoop.h"

namespace doggy
{
        namespace net
        {
                class EventLoopThread
                {
                        using ThreadInitCallback = std::function<void(EventLoop *)>;

                public:
                        EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());
                        ~EventLoopThread();
                        EventLoop *startLoop();
                        bool started_() { return start_.load(std::memory_order_relaxed); };

                private:
                        void threadFunc();

                        EventLoop *loop_;
                        std::atomic<bool> exiting_;
                        std::atomic<bool> start_;

                        std::mutex mtx_;
                        std::condition_variable cv_;
                        std::vector<std::thread> thread_;

                        ThreadInitCallback callback_;
                };

        } // namespace net
} // namespace doggy

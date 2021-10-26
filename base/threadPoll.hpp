#pragma once

#include "./header.h"

namespace doggy
{
        class ThreadPoll
        {
        public:
                explicit ThreadPoll(int numThreads = static_cast<int>(std::thread::hardware_concurrency())) : numTasks_{0}
                {
                        stop_.store(false, std::memory_order_relaxed);

                        for (int i = 0; i < numThreads; ++i)
                        {
                                threads_.emplace_back(new std::thread([&]()
                                                                      {
                                        while(!stop_.load(std::memory_order_relaxed))
                                        {
                                                std::function<void()> f;
                                                {
                                                        std::unique_lock lk(mutex_);
                                                        cv_.wait(lk, [&]()
                                                                 { return  numTasks_> 0 ; });
                                                        if(stop_.load(std::memory_order_relaxed))
                                                                break;

                                                        f = task_.front();
                                                        task_.pop();
                                                        --numTasks_;
                                                }
                                                f();
                                        } }));
                        }
                }

                template <typename F, typename... Args>
                void enqueue(F f, Args... args)
                {
                        auto task = std::bind(f, args...);
                        {
                                std::lock_guard lk(mutex_);
                                task_.push(std::move(task));
                                ++numTasks_;
                        }
                        cv_.notify_one();
                }

                ~ThreadPoll()
                {
                        stop_.store(true, std::memory_order_relaxed);
                        for (auto &t : threads_)
                                (*t).join();
                }

        private:
                std::mutex mutex_;
                std::atomic<bool> stop_;
                size_t numTasks_;
                std::condition_variable cv_;
                std::queue<std::function<void()>> task_;
                std::vector<std::unique_ptr<std::thread>> threads_;
        };

} // namespace doggy
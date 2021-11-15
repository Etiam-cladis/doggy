#pragma once

#include "./EventLoopPool.h"

using namespace doggy;
using namespace doggy::net;

EventLoopPool::EventLoopPool(unsigned int numThreads)
    : started_(false),
      numThreads_(numThreads),
      nextLoop_(0)
{
}

EventLoopPool::~EventLoopPool()
{
        started_.store(false, std::memory_order_release);
        for (auto &t : threads_)
        {
                t.join();
        }
}

void EventLoopPool::start()
{
        assert(!started_.load(std::memory_order_relaxed));

        started_.store(true, std::memory_order_relaxed);
        for (int i = 0; i < numThreads_; ++i)
        {
                std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
                loops_.emplace_back(std::move(loop));
                EventLoop *lp = loops_[i].get();
                threads_.emplace_back(std::thread([lp]()
                                                  { lp->loop(); }));
        }
}
void EventLoopPool::stopAll()
{
        assert(started_.load(std::memory_order_relaxed));

        started_.store(false, std::memory_order_release);
        for (auto &&loop : loops_)
        {
                loop->quit();
        }
}

EventLoop *EventLoopPool::getNextLoop()
{
        assert(started_.load(std::memory_order_relaxed));

        EventLoop *lp = nullptr;
        if (!loops_.empty())
        {
                lp = loops_[nextLoop_].get();
                ++nextLoop_;
                nextLoop_ %= numThreads_;
        }
        return lp;
}

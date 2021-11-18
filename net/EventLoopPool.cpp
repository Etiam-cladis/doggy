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
}

void EventLoopPool::start(const ThreadInitCallback &cb)
{
        assert(!started_.load(std::memory_order_relaxed));

        started_.store(true, std::memory_order_relaxed);
        for (int i = 0; i < numThreads_; ++i)
        {
                std::unique_ptr<EventLoopThread> lp = std::make_unique<EventLoopThread>(cb);
                loops_.push_back(lp->startLoop());
                threads_.emplace_back(std::move(lp));
        }
}

EventLoop *EventLoopPool::getNextLoop()
{
        assert(started_.load(std::memory_order_relaxed));

        EventLoop *lp = nullptr;
        if (!loops_.empty())
        {
                lp = loops_[nextLoop_];
                ++nextLoop_;
                nextLoop_ %= numThreads_;
        }
        return lp;
}

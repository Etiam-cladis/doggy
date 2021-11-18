#pragma once

#include "./EventLoopThread.h"

using namespace doggy;
using namespace doggy::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
    : loop_(nullptr),
      exiting_(false),
      start_(false),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
        exiting_.store(true, std::memory_order_release);
        if (loop_ != nullptr)
        {
                loop_->quit();
                (*thread_.begin()).join();
        }
}

EventLoop *EventLoopThread::startLoop()
{
        assert(!start_.load(std::memory_order_relaxed));
        thread_.emplace_back([this]()
                             { this->threadFunc(); });

        EventLoop *loop = nullptr;
        {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this]()
                         { return this->started_(); });
                loop = loop_;
        }

        return loop;
}

void EventLoopThread::threadFunc()
{
        EventLoop loop;

        if (callback_)
        {
                callback_(&loop);
        }

        {
                std::lock_guard<std::mutex> lk(mtx_);
                loop_ = &loop;
        }
        start_.store(true, std::memory_order_release);
        cv_.notify_one();

        loop.loop();
        {
                std::lock_guard<std::mutex> lk(mtx_);
                loop_ = nullptr;
        }
}


#include "./EventLoop.h"

using namespace doggy;
using namespace doggy::net;

void EventLoop::doPendingFunctors()
{
        std::vector<Functor> temp;

        callPending_.store(true, std::memory_order_release);
        {
                std::lock_guard<std::mutex> lk(pendingQueueMutex_);
                temp.swap(pendingQueue_);
        }

        for (auto &cb : temp)
        {
                cb();
        }

        callPending_.store(false, std::memory_order_relaxed);
}
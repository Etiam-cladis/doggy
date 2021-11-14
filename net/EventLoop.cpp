
#include "./EventLoop.h"

using namespace doggy;
using namespace doggy::net;

namespace
{
        __thread EventLoop *tloopInThisThread = nullptr;

        int creatEventFd()
        {
                int fd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
                if (fd < 0)
                {
                        // LOG SYSERR TODO
                        abort();
                }
                return fd;
        }
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      threadId_(std::this_thread::get_id()),
      poller_(std::make_unique<Epoll>(this)),
      wakeupFd_(creatEventFd()),
      wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
      currentChannel_(nullptr),
      callPending_(false)
{
        if (tloopInThisThread)
        {
                // LOG FATAL TODO
                abort();
        }
        else
        {
                tloopInThisThread = this;
        }
        wakeupChannel_->setReadCallback([this]()
                                        { this->handleRead(); });
        wakeupChannel_->enableRead();
}

EventLoop::~EventLoop()
{
        wakeupChannel_->disableAll();
        wakeupChannel_->remove();
        ::close(wakeupFd_);
        tloopInThisThread = nullptr;
}

void EventLoop::loop()
{
}

void EventLoop::doPendingFunctors()
{
        std::vector<Functor> temp;

        callPending_.store(true, std::memory_order_relaxed);
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
void EventLoop::wakeup()
{
        uint64_t one = 1;
        ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
        if (n != sizeof(one))
        {
                // LOG ERROR  TODO
        }
}

void EventLoop::handleRead()
{
        uint64_t one = 1;
        ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
        if (n != sizeof(one))
        {
                // LOG ERROR  TODO
        }
}

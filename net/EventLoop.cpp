
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
                                        { this->wakeupReadHanding(); });
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
        assert(!looping_);
        assertInLoopThread();
        looping_.store(true, std::memory_order_relaxed);
        if (quit_.load(std::memory_order_acquire) == false)
        {
                while (!quit_.load(std::memory_order_relaxed))
                {
                        activeChannelList_.clear();
                        poller_->poll(10000 /* time class  TODO*/, activeChannelList_);

                        eventHandling_.store(true, std::memory_order_release);
                        for (auto *channel : activeChannelList_)
                        {
                                currentChannel_ = channel;
                                currentChannel_->handleEvent();
                        }
                        currentChannel_ = nullptr;
                        eventHandling_.store(false, std::memory_order_relaxed);
                        doPendingFunctors();
                }
        }
        looping_.store(false, std::memory_order_relaxed);
}

void EventLoop::quit()
{
        quit_.store(true, std::memory_order_release);
        if (!isInLoopThread())
        {
                wakeup();
        }
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

void EventLoop::wakeupReadHanding()
{
        uint64_t one = 1;
        ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
        if (n != sizeof(one))
        {
                // LOG ERROR  TODO
        }
}

void EventLoop::runInLoop(const Functor &cb)
{
        if (isInLoopThread())
        {
                cb();
        }
        else
        {
                queueInLoop(cb);
        }
}

void EventLoop::runInLoop(Functor &&cb)
{
        if (isInLoopThread())
        {
                cb();
        }
        else
        {
                queueInLoop(std::move(cb));
        }
}

void EventLoop::queueInLoop(const Functor &cb)
{
        {
                std::lock_guard<std::mutex> lk(pendingQueueMutex_);
                pendingQueue_.emplace_back(cb);
        }

        if (!isInLoopThread() || callPending_.load(std::memory_order_relaxed))
        {
                wakeup();
        }
}

void EventLoop::queueInLoop(Functor &&cb)
{
        {
                std::lock_guard<std::mutex> lk(pendingQueueMutex_);
                pendingQueue_.emplace_back(std::move(cb));
        }

        if (!isInLoopThread() || callPending_.load(std::memory_order_relaxed))
        {
                wakeup();
        }
}

size_t EventLoop::queueSize() const
{
        std::lock_guard<std::mutex> lk(pendingQueueMutex_);
        return pendingQueue_.size();
}

void EventLoop::updateChannelRW(Channel *channel)
{
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        poller_->updateChannelRW(channel);
}

void EventLoop::updateChannelOther(Channel *channel)
{
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        poller_->updateChannelOther(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        if (eventHandling_.load(std::memory_order_relaxed))
        {
                assert(currentChannel_ == channel ||
                       std::find(activeChannelList_.begin(), activeChannelList_.end(), channel) == activeChannelList_.end());
        }
        poller_->removeChannel(channel);
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
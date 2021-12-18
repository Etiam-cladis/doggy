#include "doggy/net/Channel.h"

#include "doggy/net/Channel.h"
#include "doggy/net/EventLoop.h"

using namespace doggy;
using namespace doggy::net;

Channel::Channel(EventLoop *loop, int fd)
    : loop_{loop},
      fd_{fd},
      event_{0},
      rEvent_{0},
      index_{-1},
      tied_(false),
      eventHading_{false},
      addedToLoop_{false}
{
}

Channel::~Channel()
{
        assert(!eventHading_);
        assert(!addedToLoop_);
}

void Channel::updateRW()
{
        addedToLoop_ = true;
        loop_->updateChannelRW(this);
}

void Channel::remove()
{
        assert(isNoneRWEvent());
        addedToLoop_ = false;
        loop_->removeChannel(this);
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
        tie_ = obj;
        tied_ = true;
}

void Channel::handleEvent()
{
        std::shared_ptr<void> guard;
        if (tied_)
        {
                guard = tie_.lock();
                handleEventWithGuard();
        }
        else
        {
                handleEventWithGuard();
        }
}

void Channel::handleEventWithGuard()
{
#ifndef USE_LT_MODE
        eventHading_ = true;

        if (rEvent_ & EPOLLHUP && ~(rEvent_ & EPOLLIN))
        {
                if (closeCallback_)
                        closeCallback_();
        }

        if (rEvent_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {
                if (readCallback_)
                        readCallback_();
        }

        if (rEvent_ & EPOLLOUT)
        {
                if (writeCallback_)
                        writeCallback_();
        }

        if (rEvent_ & EPOLLERR)
        {
                if (errorCallback_)
                        errorCallback_();
        }

        eventHading_ = false;
#endif
}

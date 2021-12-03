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

void Channel::updateOther()
{
        loop_->updateChannelOther(this);
}

void Channel::remove()
{
        assert(isNoneRWEvent());
        addedToLoop_ = false;
        loop_->removeChannel(this);
}

void Channel::handleEvent()
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

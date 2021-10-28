#include "./Channel.h"

using namespace doggy;
using namespace doggy::net;

Channel::Channel(const std::shared_ptr<doggy::base::EventLoop> &loop, int fd)
    : loop_{loop},
      fd_{fd},
      event_{kNoneEvent},
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

void Channel::update()
{
        addedToLoop_ = true;
        if (auto spt = loop_.lock())
        {
                spt->updateChannel(this);
        }
        else
        {
                std::cerr << "loop expired" << std::endl;
                abort();
        }
}

void Channel::remove()
{
        assert(isNoneEvent());
        addedToLoop_ = false;
        if (auto spt = loop_.lock())
        {
                spt->removeChannel(this);
        }
        else
        {
                std::cerr << "loop expired" << std::endl;
                abort();
        }
}

void Channel::handleEvent()
{
        eventHading_ = true;
        // tomorrow
        eventHading_ = false;
}

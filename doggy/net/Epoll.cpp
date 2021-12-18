#include "doggy/net/Epoll.h"

#include "doggy/net/Channel.h"

using namespace doggy;
using namespace doggy::net;

namespace
{
        const int kNew = -1;
        const int kAdded = 1;
        const int kDeleted = 2;
}

Epoll::Epoll(EventLoop *loop)
    : loop_(loop),
      epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
      eventList_(kinitEventListSize)
{
        if (epollFd_ < 0)
        {
                // LOG FATAL TODO
                abort();
        }
}

Epoll::~Epoll()
{
        ::close(epollFd_);
}

void Epoll::poll(int timesoutMs, ChannelList &channelList)
{
        int numEvents = ::epoll_wait(epollFd_, &*eventList_.begin(), static_cast<int>(eventList_.size()), timesoutMs);
        int saveError = errno;

        if (numEvents > 0)
        {
                fillActiveChannels(numEvents, channelList);
                if (static_cast<size_t>(numEvents) == eventList_.size())
                {
                        eventList_.resize(eventList_.size() * 2);
                }
        }
        else if (numEvents == 0)
        {
                // LOG  TRACE NOTHING HAPPENED
        }
        else
        {
                if (saveError != EINTR)
                {
                        // LOG
                        abort();
                }
        }
}

void Epoll::fillActiveChannels(int numEvents, ChannelList &activeChannelList) const
{
        for (int i = 0; i < numEvents; ++i)
        {
                Channel *channel = static_cast<Channel *>(eventList_[i].data.ptr);
                int fd = channel->fd();
                if (channels_.find(fd) != channels_.end())
                {
                        channel->setRevents(eventList_[i].events);
                        activeChannelList.push_back(channel);
                }
                else
                {
                        // LOG CHANNEL DOESN'T EXIST
                }
        }
}

void Epoll::updateChannelRW(Channel *channel)
{
        assertInLoopThread();
        int index = channel->index();
        if (index == kNew || index == kDeleted)
        {
                int fd = channel->fd();
                if (index == kNew)
                {
                        assert(channels_.find(fd) == channels_.end());
                        channels_[fd] = channel;
                }
                else
                {
                        assert(channels_.find(fd) != channels_.end());
                        assert(channels_[fd] == channel);
                }
                channel->setIndex(kAdded);
                update(EPOLL_CTL_ADD, channel);
        }
        else
        {
                int fd = channel->fd();
                assert(channels_.find(fd) != channels_.end());
                assert(channels_[fd] == channel);
                assert(index == kAdded);
                if (channel->isNoneRWEvent())
                {
                        update(EPOLL_CTL_DEL, channel);
                        index = kDeleted;
                }
                else
                {
                        update(EPOLL_CTL_MOD, channel);
                }
        }
}

void Epoll::removeChannel(Channel *channel)
{
        assertInLoopThread();
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(channel->isNoneRWEvent());
        int index = channel->index();
        assert(index == kAdded || index == kDeleted);
        size_t n = channels_.erase(fd);
        assert(n == 1);

        if (index == kAdded)
        {
                update(EPOLL_CTL_DEL, channel);
        }
        channel->setIndex(kNew);
}

void Epoll::update(int operation, Channel *channel)
{
        assertInLoopThread();
        epoll_event event{0};
        event.events = channel->events();
        event.data.ptr = channel;
        int fd = channel->fd();

        if (::epoll_ctl(epollFd_, operation, fd, &event) < 0)
        {
                if (operation != EPOLL_CTL_DEL)
                {
                        // LOG SYSFATAL TODO
                        abort();
                }
        }
}

void Epoll::assertInLoopThread() const
{
        loop_->assertInLoopThread();
}
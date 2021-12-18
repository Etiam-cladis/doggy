#ifndef DOGGY_NET_EPOLL_H
#define DOGGY_NET_EPOLL_H

#include "doggy/net/header.h"

#include "doggy/net/EventLoop.h"

namespace doggy
{
        namespace net
        {
                class Channel;

                class Epoll
                {
                        using EventList = std::vector<epoll_event>;
                        using ChannelList = std::vector<Channel *>;
                        using ChannelMap = std::unordered_map<int, Channel *>;

                public:
                        Epoll(EventLoop *loop);
                        ~Epoll();

                public:
                        Epoll(const Epoll &) = delete;
                        Epoll &operator=(const Epoll &) = delete;

                public:
                        void poll(int timesoutMs, ChannelList &channelList);
                        void updateChannelRW(Channel *channel);
                        void removeChannel(Channel *channel);

                private:
                        void fillActiveChannels(int numEvents, ChannelList &activeChannels) const;
                        void update(int operation, Channel *channel);
                        void assertInLoopThread() const;

                private:
                        static const int kinitEventListSize = 16;

                        EventLoop *loop_;
                        int epollFd_;
                        EventList eventList_;
                        ChannelMap channels_;
                };

        } // namespace net

} // namespace doggy

#endif // DOGGY_NET_EPOLL_H

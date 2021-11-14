#pragma once

#include "../base/header.h"

#include "./Channel.h"

namespace doggy
{
        namespace net
        {
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
                        void updateChannelOther(Channel *channel);
                        void updateChannelRW(Channel *channel);
                        void removeChannel(Channel *channel);

                private:
                        void fillActiveChannels(int numEvents, ChannelList &activeChannels) const;
                        void update(int operation, Channel *channel);
                        void assertInLoopThread() const
                        {
                                loop_->assertInLoopThread();
                        }

                private:
                        static const int kinitEventListSize = 16;

                        EventLoop *loop_;
                        int epollFd_;
                        EventList eventList_;
                        ChannelMap channels_;
                };

        } // namespace net

} // namespace doggy
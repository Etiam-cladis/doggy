#pragma once

#include "./header.h"

#include "./Channel.h"
#include "./Epoll.h"

namespace doggy
{
        namespace net
        {
                class EventLoop
                {
                        using Functor = std::function<void()>;
                        using ChannelList = std::vector<Channel *>;

                public:
                        EventLoop();
                        ~EventLoop();

                public:
                        EventLoop(const EventLoop &) = delete;
                        EventLoop &operator=(const EventLoop &) = delete;

                public:
                        void loop();
                        void quit();
                        void wakeup();
                        void handleRead();

                        void runInLoop(const Functor &cb);
                        void runInLoop(Functor &&cb);
                        void queueInLoop(const Functor &cb);
                        void queueInLoop(Functor &&cb);

                        void updateChannelRW(Channel *channel);
                        void updateChannelOther(Channel *channel);

                        void removeChannel(Channel *channel);

                        void assertInLoopThread()
                        {
                                if (!isInLoopThread())
                                        abortNotInThreadLoop();
                        }
                        bool isInLoopThread() { return threadId_ == std::this_thread::get_id(); }

                private:
                        void abortNotInThreadLoop();
                        void wakeupReadHading();
                        void doPendingFunctors();

                private:
                        std::atomic<bool> looping_;
                        std::atomic<bool> quit_;
                        std::atomic<bool> eventHandling_;
                        std::thread::id threadId_;

                        std::unique_ptr<Epoll> poller_;

                        int wakeupFd_;
                        std::unique_ptr<Channel> wakeupChannel_;

                        ChannelList channelList_;
                        Channel *currentChannel_;

                        std::atomic<bool> callPending_;
                        std::mutex pendingQueueMutex_;
                        std::vector<Functor> pendingQueue_;
                };

        } // namespace net

} // namespace doggy
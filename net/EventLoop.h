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

                        void runInLoop(const Functor &cb);
                        void runInLoop(Functor &&cb);
                        void queueInLoop(const Functor &cb);
                        void queueInLoop(Functor &&cb);
                        size_t queueSize() const;

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
                        void abortNotInThreadLoop()
                        {
                                // LOG FATAL TODO
                                abort();
                        }

                        void wakeupReadHanding();
                        void doPendingFunctors();

                private:
                        std::atomic<bool> looping_;
                        std::atomic<bool> quit_;
                        std::atomic<bool> eventHandling_;
                        std::thread::id threadId_;

                        std::unique_ptr<Epoll> poller_;

                        int wakeupFd_;
                        std::unique_ptr<Channel> wakeupChannel_;

                        ChannelList activeChannelList_;
                        Channel *currentChannel_;

                        std::atomic<bool> callPending_;
                        mutable std::mutex pendingQueueMutex_;
                        std::vector<Functor> pendingQueue_;
                };

        } // namespace net

} // namespace doggy
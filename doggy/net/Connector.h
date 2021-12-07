#ifndef DOGGY_NET_CONNECTOR_H
#define DOGGY_NET_CONNECTOR_H

#include "doggy/net/header.h"

#include "doggy/net/InetAddress.h"
#include "doggy/net/TimerId.h"
#include "doggy/net/TimerQueue.h"

namespace doggy
{
        namespace net
        {
                class Channel;
                class EventLoop;

                using namespace std::chrono_literals;
                class Connector
                {
                        using NewConnectionCallback = std::function<void(int sockFd)>;

                public:
                        Connector(EventLoop *loop, const InetAddress &serverAddr);
                        ~Connector();

                public:
                        Connector(const Connector &) = delete;
                        Connector &operator=(const Connector &) = delete;

                public:
                        void setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; };
                        void setNewConnectionCallback(NewConnectionCallback &&cb) { newConnectionCallback_ = std::move(cb); };

                        const InetAddress &getServerAddr() const { return serverAddr_; };

                        void start();
                        void stop();
                        void restart();

                private:
                        enum States
                        {
                                kDisconnected,
                                kConnecting,
                                kConnected
                        };
                        void setState(States s) { state_.store(s, std::memory_order_relaxed); };
                        void startInLoop();
                        void stopInLoop();
                        void connect();
                        void connecting(int sockfd);
                        void handleWrite();
                        void handleError();
                        void retry(int sockfd);
                        int removeAndResetChannel();
                        void resetChannel();

                private:
                        static constexpr std::chrono::milliseconds kMaxRetryDelayMs = 30 * 1000ms;
                        static constexpr std::chrono::milliseconds kInitRetryDelayMs = 500ms;

                        EventLoop *loop_;
                        TimerQueue timerQueue_;
                        TimerId cancelTimerId_;
                        InetAddress serverAddr_;
                        std::atomic<bool> connect_;
                        std::atomic<States> state_;
                        std::unique_ptr<Channel> channel_;
                        NewConnectionCallback newConnectionCallback_;
                        std::chrono::milliseconds retryDelayMs_;
                };

        } // namespace net

} // namespace doggy

#endif // DOGGY_NET_CONNECTOR_H

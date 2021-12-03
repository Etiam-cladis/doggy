#ifndef DOGGY_NET_TCPCLIENT_H
#define DOGGY_NET_TCPCLIENT_H

#include "doggy/net/header.h"

#include "doggy/net/TcpConnection.h"

namespace doggy
{
        namespace net
        {
                class Connector;

                class TcpClient
                {
                        using ConnectorPtr = std::unique_ptr<Connector>;
                        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
                        using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
                        using MessageCallback = std::function<void(const TcpConnectionPtr &, Buff &buff)>;

                public:
                        TcpClient(EventLoop *loop, const InetAddress &serverAddr);
                        ~TcpClient();

                public:
                        explicit TcpClient(const TcpClient &) = delete;
                        TcpClient &operator=(const TcpClient &) = delete;

                public:
                        void connect();
                        void disconnect();
                        void stop();

                        TcpConnectionPtr connection() const
                        {
                                std::lock_guard<std::mutex> lk(mutex_);
                                return connection_;
                        }

                        EventLoop *getLoop() const { return loop_; }
                        bool retry() const { return retry_.load(std::memory_order_relaxed); }
                        void enableRetry() { retry_.store(true, std::memory_order_relaxed); }

                        void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
                        void setConnectionCallback(ConnectionCallback &&cb) { connectionCallback_ = std::move(cb); }
                        void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
                        void setMessageCallback(MessageCallback &&cb) { messageCallback_ = std::move(cb); }

                private:
                        void newConnection(int sockFd);
                        void removeConnectiion(const TcpConnectionPtr &conn);

                private:
                        EventLoop *loop_;
                        ConnectorPtr connector_;
                        ConnectionCallback connectionCallback_;
                        MessageCallback messageCallback_;
                        std::atomic<bool> retry_;
                        std::atomic<bool> connect_;

                        mutable std::mutex mutex_;
                        TcpConnectionPtr connection_;
                };

        } // namespace net

} // namespace doggy

#endif // DOGGY_NET_TCPCLIENT_H

#ifndef DOGGY_NET_TCPCONNECTION_H
#define DOGGY_NET_TCPCONNECTION_H

#include "doggy/net/header.h"

#include "doggy/net/Buff.h"
#include "doggy/net/InetAddress.h"

namespace doggy
{
        namespace net
        {
                class Channel;
                class EventLoop;
                class Socket;

                class TcpConnection : public std::enable_shared_from_this<TcpConnection>
                {
                        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
                        using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
                        using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
                        using MessageCallback = std::function<void(const TcpConnectionPtr &, Buff &buff)>;

                public:
                        TcpConnection(EventLoop *loop, int sockFd, const InetAddress &localAddr, const InetAddress &peerAddr);
                        ~TcpConnection();

                public:
                        TcpConnection(const TcpConnection &) = delete;
                        TcpConnection &operator=(const TcpConnection &) = delete;

                public:
                        EventLoop *getLoop() const { return loop_; };
                        const InetAddress &getLocalAddr() const { return localAddr_; };
                        const InetAddress &getPeerAddr() const { return peerAddr_; };
                        bool connected() const { return tcpConnectionState_.load(std::memory_order_relaxed) == kConnected; };
                        bool disconnected() const { return tcpConnectionState_.load(std::memory_order_relaxed) == kDisconnected; };
                        bool getTcpInfo(tcp_info *) const;

                public:
                        void send(const std::string &message);
                        void send(std::string &&message);
                        void send(Buff &message);

                        void shutdownWrite();
                        void forceClose();

                        bool isReading() const { return reading_.load(std::memory_order_acquire); }
                        void startRead();
                        void stopRead();

                        void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
                        void setConnectionCallback(ConnectionCallback &&cb) { connectionCallback_ = std::move(cb); }
                        void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
                        void setMessageCallback(MessageCallback &&cb) { messageCallback_ = std::move(cb); }
                        void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
                        void setCloseCallback(CloseCallback &&cb) { closeCallback_ = std::move(cb); }

                        void connectEstablished();
                        void connectDestroyed();

                private:
                        void handleRead();
                        void handleWrite();
                        void handleClose();
                        void handleError();

                        void sendInLoop(const std::string &message);
                        void sendInLoop(std::string &&message);
                        void sendInLoop(const void *buf, size_t len);

                        void shutdownInLoop();
                        void forceCloseInLoop();

                        void startReadInLoop();
                        void stopReadInLoop();

                        enum State
                        {
                                kDisconnected,
                                kConnecting,
                                kConnected,
                                kDisconnecting
                        };
                        void setState(State s) { tcpConnectionState_.store(s, std::memory_order_relaxed); }

                private:
                        EventLoop *loop_;
                        std::atomic<State> tcpConnectionState_;
                        std::atomic<bool> reading_;

                        const InetAddress localAddr_;
                        const InetAddress peerAddr_;

                        std::unique_ptr<Socket> socket_;
                        std::unique_ptr<Channel> channel_;

                        ConnectionCallback connectionCallback_;
                        CloseCallback closeCallback_;
                        MessageCallback messageCallback_;

                        Buff outputBuffer_;
                        Buff inputBuffer_;
                };

        } // namespace net
} // namespace doggy

#endif // DOGGY_NET_TCPCONNECTION_H

#pragma once

#include "./header.h"

#include "./EventLoop.h"
#include "./Channel.h"
#include "./Socket.h"
#include "./InetAddr.h"

namespace doggy
{
        namespace net
        {
                class Acceptor
                {
                        using newConnectionCallBack = std::function<void(int socket, const InetAddress &)>;

                public:
                        Acceptor(EventLoop *loop, const InetAddress &lisenAddr, bool reuseport);
                        ~Acceptor();

                public:
                        Acceptor(const Acceptor &) = delete;
                        Acceptor &operator=(const Acceptor &) = delete;

                public:
                        void setNewConnectionCallBack(const newConnectionCallBack &cb) { newConnectionCallBack_ = cb; };
                        void setNewConnectionCallBack(newConnectionCallBack &&cb) { newConnectionCallBack_ = std::move(cb); };

                        bool listenning() { return listenning_.load(std::memory_order_relaxed); };
                        void listen();

                private:
                        void handleRead();

                private:
                        EventLoop *loop_;
                        Socket acceptSockId_;
                        Channel acceptChannel_;
                        newConnectionCallBack newConnectionCallBack_;
                        std::atomic<bool> listenning_;
                };

        } // namespace net
} // namespace doggy

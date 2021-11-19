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
                public:
                        Acceptor(const Acceptor &) = delete;
                        Acceptor &operator=(const Acceptor &) = delete;

                private:
                        void handleRead();

                private:
                        EventLoop *loop_;
                        Socket acceptSockId_;
                        Channel acceptChannel_;
                        newConnectionCallBack newConnectionCallBack_;
                };

        } // namespace net
} // namespace doggy

#ifndef EXAMPLE_ECHO_ECHO_SERVER_H
#define EXAMPLE_ECHO_ECHO_SERVER_H

#include "doggy/net/TcpServer.h"

using namespace doggy;
using namespace doggy::net;

class EchoServer
{
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

public:
        EchoServer(doggy::net::EventLoop *loop,
                   const doggy::net::InetAddress &inetaddr,
                   int numThreads);

public:
        void start();

private:
        void onConnection(const TcpConnectionPtr &conn);
        void onMessage(const TcpConnectionPtr &conn,
                       Buff &buff);

private:
        doggy::net::TcpServer echoServer_;
        std::atomic<int> numConnected_;
};

#endif // EXAMPLE_ECHO_ECHO_SERVER_H

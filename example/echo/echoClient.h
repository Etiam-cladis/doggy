#ifndef EXAMPLE_ECHO_ECHO_CLIENT_H
#define EXAMPLE_ECHO_ECHO_CLIENT_H

#include "doggy/net/TcpClient.h"
#include "doggy/net/EventLoop.h"

using namespace doggy;
using namespace doggy::net;

class EchoClient
{
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

public:
        EchoClient(EventLoop *loop, const InetAddress &serverAddr, std::string message);

public:
        void connect();

private:
        void onConnection(const TcpConnectionPtr &conn);
        void onMessage(const TcpConnectionPtr &conn, Buff &buff);

private:
        EventLoop *loop_;
        TcpClient echoClient_;
        std::string message_;
};

#endif // EXAMPLE_ECHO_ECHO_CLIENT_H

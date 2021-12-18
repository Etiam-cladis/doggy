#include "./echoServer.h"

using namespace doggy;
using namespace doggy::net;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

EchoServer::EchoServer(EventLoop *loop,
                       const InetAddress &inetaddr,
                       int numThreads)
    : echoServer_(loop, inetaddr),
      numConnected_(0)
{
        echoServer_.setNumThreads(numThreads);
        echoServer_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                          { this->onConnection(conn); });
        echoServer_.setMessageCallback([this](const TcpConnectionPtr &conn, Buff &buff)
                                       { this->onMessage(conn, buff); });
}

void EchoServer::start()
{
        echoServer_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr &conn)
{
        if (conn->connected())
        {
                conn->setTcpNoDelay(true);
                ++numConnected_;
                std::cout << "numConnected_:" << numConnected_ << std::endl;
        }
        else
        {
                --numConnected_;
        }
}

void EchoServer::onMessage(const TcpConnectionPtr &conn, Buff &buff)
{
        std::string msg(buff.retrieveAllAsString());
        std::cout << "echoServer receive" << msg.size() << "Bytes" << std::endl;
        std::cout << "echo:" << msg << std::endl;
        conn->send(msg);
}
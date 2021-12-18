#include "./echoClient.h"

using namespace doggy;
using namespace doggy::net;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

EchoClient::EchoClient(EventLoop *loop, const InetAddress &serverAddr, std::string message)
    : loop_(loop),
      echoClient_(loop, serverAddr),
      message_(message)
{
        echoClient_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                          { this->onConnection(conn); });
        echoClient_.setMessageCallback([this](const TcpConnectionPtr &conn, Buff &buff)
                                       { this->onMessage(conn, buff); });
}

void EchoClient::connect()
{
        echoClient_.connect();
}

void EchoClient::onConnection(const TcpConnectionPtr &conn)
{
        if (conn->connected())
        {
                conn->setTcpNoDelay(true);
                std::cout << "sned" << message_.size() << "Bytes:" << message_ << std::endl;
                conn->send(message_);
                std::cout << "sned over" << std::endl;
        }
        else
        {
                loop_->quit();
        }
}

void EchoClient::onMessage(const TcpConnectionPtr &conn, Buff &buff)
{
        conn->send(buff);
}
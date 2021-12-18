#include "doggy/net/TcpClient.h"

#include "doggy/net/Connector.h"
#include "doggy/net/EventLoop.h"
#include "doggy/net/SocketsOps.h"

using namespace doggy;
using namespace doggy::net;

using ConnectorPtr = std::unique_ptr<Connector>;

TcpClient::TcpClient(EventLoop *loop, const InetAddress &sersverAddr)
    : loop_(loop),
      connector_(std::make_unique<Connector>(loop, sersverAddr)),
      retry_(false),
      connect_(true)
{
        connector_->setNewConnectionCallback([this](int sockFd)
                                             { this->newConnection(sockFd); });
}

TcpClient::~TcpClient()
{
        TcpConnectionPtr conn;
        bool unique = false;
        {
                std::lock_guard<std::mutex> lk(mutex_);
                unique = connection_.unique();
                conn = connection_;
        }
        if (conn)
        {
                assert(loop_ == conn->getLoop());
                auto p = conn.get();

                loop_->runInLoop([this, conn = std::move(conn)]()
                                 { conn->setCloseCallback([this, conn](const TcpConnectionPtr &)
                                                          { removeConnectiion(conn); }); });
                if (unique)
                {
                        p->forceClose();
                }
        }
        else
        {
                connector_->stop();
        }
}

void TcpClient::connect()
{
        connect_.store(true, std::memory_order_relaxed);
        connector_->start();
}

void TcpClient::disconnect()
{
        connect_.store(false, std::memory_order_relaxed);
        {
                std::lock_guard<std::mutex> lk(mutex_);
                if (connection_)
                {
                        connection_->shutdownWrite();
                }
        }
}

void TcpClient::stop()
{
        connect_.store(false, std::memory_order_relaxed);
        connector_->stop();
}

void TcpClient::newConnection(int sockFd)
{
        loop_->assertInLoopThread();

        InetAddress localAddr(SocketOps::getLocalAddr(sockFd));
        InetAddress peerAddr(SocketOps::getPeerAddr(sockFd));
        TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop_, sockFd, localAddr, peerAddr));
        conn->setConnectionCallback(connectionCallback_);
        conn->setMessageCallback(messageCallback_);
        conn->setCloseCallback([this, conn](const TcpConnectionPtr &)
                               { this->removeConnectiion(conn); });

        {
                std::lock_guard<std::mutex> lk(mutex_);
                connection_ = conn;
        }
        conn->connectEstablished();
}

void TcpClient::removeConnectiion(const TcpConnectionPtr &conn)
{

        loop_->assertInLoopThread();
        assert(loop_ == conn->getLoop());

        {
                std::lock_guard<std::mutex> lk(mutex_);
                assert(connection_ == conn);
                connection_.reset();
        }

        loop_->queueInLoop([conn]()
                           { conn->connectDestroyed(); });
        if (retry_ && connect_)
        {
                connector_->restart();
        }
}
#include "doggy/net/TcpServer.h"

#include "doggy/net/Acceptor.h"
#include "doggy/net/EventLoop.h"
#include "doggy/net/EventLoopPool.h"
#include "doggy/net/SocketsOps.h"

using namespace doggy;
using namespace doggy::net;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, Option option)
    : loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, listenAddr, option == kReusePort)),
      numThreads_(std::thread::hardware_concurrency()),
      threadPool_(std::make_shared<EventLoopPool>(numThreads_)),
      started_(false)
{
        acceptor_->setNewConnectionCallBack([this](int sockfd, const InetAddress &peerAddr)
                                            { this->newConnection(sockfd, peerAddr); });
}

TcpServer::~TcpServer()
{
}

void TcpServer::setNumThreads(int numThreads)
{
        assert(numThreads >= 0);
        if (!started_.load(std::memory_order_relaxed))
        {
                numThreads_ = numThreads;
                threadPool_->setNumThreads(numThreads_);
        }
}

void TcpServer::start()
{
        if (!started_.load(std::memory_order_relaxed))
        {
                threadPool_->start(threadInitCallback_);

                assert(!acceptor_->listenning());
                auto p = acceptor_.get();
                loop_->runInLoop([p]()
                                 { p->listen(); });
        }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
        loop_->assertInLoopThread();
        EventLoop *ioLoop = nullptr;
        if (numThreads_ == 0)
        {
                ioLoop = loop_;
        }
        else
        {
                ioLoop = threadPool_->getNextLoop();
        }

        InetAddress localAddr(SocketOps::getLocalAddr(sockfd));
        TcpConnectionPtr conn(std::make_shared<TcpConnection>(ioLoop, sockfd, localAddr, peerAddr));
        conn->setConnectionCallback(connectionCallback_);
        conn->setMessageCallback(messageCallback_);
        conn->setCloseCallback([this](const TcpConnectionPtr &conn)
                               { this->removeConnection(conn); });

        ioLoop->runInLoop([conn]()
                          { conn->connectEstablished(); });
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
        loop_->runInLoop([this, &conn]()
                         { this->removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
        loop_->assertInLoopThread();

        EventLoop *lp = conn->getLoop();
        lp->runInLoop([&conn]()
                      { conn->connectDestroyed(); });
}

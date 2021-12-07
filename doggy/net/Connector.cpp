#include "doggy/net/Connector.h"

#include "doggy/net/Channel.h"
#include "doggy/net/EventLoop.h"
#include "doggy/net/SocketsOps.h"

using namespace doggy;
using namespace doggy::net;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),
      timerQueue_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{
}

Connector::~Connector()
{
        assert(!channel_);
}

void Connector::start()
{
        connect_.store(true, std::memory_order_release);
        loop_->runInLoop([this]()
                         { this->startInLoop(); });
}

void Connector::startInLoop()
{
        loop_->assertInLoopThread();
        assert(state_.load(std::memory_order_relaxed) == kDisconnected);
        if (connect_)
        {
                connect();
        }
}

void Connector::stop()
{
        connect_.store(false, std::memory_order_release);
        if (cancelTimerId_.hasTimer())
        {
                timerQueue_.cancel(cancelTimerId_);
        }
        loop_->runInLoop([this]()
                         { this->stopInLoop(); });
}

void Connector::stopInLoop()
{
        loop_->assertInLoopThread();
        if (state_.load(std::memory_order_relaxed) == kConnecting)
        {
                setState(kConnected);
                int sockFd = removeAndResetChannel();
                retry(sockFd);
        }
}

void Connector::restart()
{
        loop_->assertInLoopThread();
        setState(kDisconnected);
        retryDelayMs_ = kInitRetryDelayMs;
        connect_.store(true, std::memory_order_release);
        startInLoop();
}

void Connector::connect()
{
        int sockfd = SocketOps::createNonblockingOrDie(serverAddr_.getFamily());
        int ret = SocketOps::connect(sockfd, serverAddr_.getSockaddr());
        int savedErrno = (ret == 0) ? 0 : errno;
        switch (savedErrno)
        {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
                connecting(sockfd);
                break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
                retry(sockfd);
                break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
                SocketOps::close(sockfd);
                break;

        default:
                SocketOps::close(sockfd);
                break;
        }
}

void Connector::connecting(int sockFd)
{
        setState(kConnecting);
        assert(!channel_);
        channel_.reset(new Channel(loop_, sockFd));
        channel_->setWriteCallback([this]()
                                   { this->handleWrite(); });
        channel_->setErrorCallback([this]()
                                   { this->handleError(); });
        channel_->enableWrite();
        channel_->enableEt();
}

int Connector::removeAndResetChannel()
{
        channel_->disableAll();
        channel_->remove();
        int sockFd = channel_->fd();

        loop_->runInLoop([this]()
                         { this->resetChannel(); });

        return sockFd;
}
void Connector::resetChannel()
{
        channel_.reset();
}
void Connector::handleWrite()
{

        if (state_.load(std::memory_order_relaxed) == kConnecting)
        {
                int sockfd = removeAndResetChannel();
                int err = SocketOps::getSocketError(sockfd);
                if (err)
                {
                        retry(sockfd);
                }
                else if (SocketOps::isSelfConnect(sockfd))
                {
                        retry(sockfd);
                }
                else
                {
                        setState(kConnected);
                        if (connect_)
                        {
                                newConnectionCallback_(sockfd);
                        }
                        else
                        {
                                SocketOps::close(sockfd);
                        }
                }
        }
        else
        {
                assert(state_ == kDisconnected);
                // LOG stop the connection before the connection  is successful
        }
}

void Connector::handleError()
{
        if (state_.load(std::memory_order_relaxed) == kConnecting)
        {
                int sockfd = removeAndResetChannel();
                [[maybe_unused]] int err = SocketOps::getSocketError(sockfd);
                // LOG err
                retry(sockfd);
        }
}

void Connector::retry(int sockFd)
{
        SocketOps::close(sockFd);
        setState(kDisconnected);
        if (connect_.load(std::memory_order_relaxed))
        {
                cancelTimerId_ = loop_->runAfter(retryDelayMs_, [this]()
                                                 { this->startInLoop(); });

                retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
        }
}

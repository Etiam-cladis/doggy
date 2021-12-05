#include "doggy/net/TcpConnection.h"

#include "doggy/net/Channel.h"
#include "doggy/net/EventLoop.h"
#include "doggy/net/Socket.h"
#include "doggy/net/SocketsOps.h"

using namespace doggy;
using namespace doggy::net;

TcpConnection::TcpConnection(EventLoop *loop, int sockFd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(loop),
      tcpConnectionState_(kConnecting),
      reading_(true),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      socket_(std::make_unique<Socket>(sockFd)),
      channel_(std::make_unique<Channel>(loop, sockFd))
{
        auto p = shared_from_this();
        channel_->setReadCallback([p]()
                                  { p->handleRead(); });
        channel_->setWriteCallback([p]()
                                   { p->handleWrite(); });
        channel_->setCloseCallback([p]()
                                   { p->handleClose(); });
        channel_->setErrorCallback([p]()
                                   { p->handleError(); });

        channel_->enableRdhup();

        socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
        assert(tcpConnectionState_.load(std::memory_order_relaxed) == kDisconnected);
}

bool TcpConnection::getTcpInfo(tcp_info *tcpinfo) const
{
        return socket_->getTcpInfo(tcpinfo);
}

void TcpConnection::send(const std::string &message)
{
        if (tcpConnectionState_.load(std::memory_order_relaxed) == kConnected)
        {
                if (loop_->isInLoopThread())
                {
                        sendInLoop(message);
                }
                else
                {
                        auto self = shared_from_this();
                        loop_->runInLoop([self, &message]()
                                         { self->sendInLoop(message); });
                }
        }
}

void TcpConnection::send(std::string &&message)
{
        if (tcpConnectionState_.load(std::memory_order_relaxed) == kConnected)
        {
                if (loop_->isInLoopThread())
                {
                        sendInLoop(std::move(message));
                }
                else
                {
                        auto self = shared_from_this();
                        loop_->runInLoop([message = std::move(message), self]() mutable
                                         { self->sendInLoop(std::move(message)); });
                }
        }
}

void TcpConnection::send(Buff &buff)
{
        if (tcpConnectionState_.load(std::memory_order_relaxed) == kConnected)
        {
                if (loop_->isInLoopThread())
                {
                        sendInLoop(buff.peek(), buff.readableBytes());
                        buff.retrieveAll();
                }
                else
                {
                        auto self = shared_from_this();
                        std::string str(std::move(buff.retrieveAllAsString()));
                        loop_->runInLoop([str = std::move(str), self]() mutable
                                         { self->sendInLoop(str); });
                }
        }
}

void TcpConnection::sendInLoop(const std::string &message)
{
        sendInLoop(message.data(), message.length());
}

void TcpConnection::sendInLoop(std::string &&message)
{
        sendInLoop(message.data(), message.length());
}

void TcpConnection::sendInLoop(const void *buf, size_t len)
{
        loop_->assertInLoopThread();
        ssize_t nwrote = 0;

        if (channel_->isWriting() && outputBuffer_.readableBytes() == 0)
        {
                nwrote = ::send(channel_->fd(), buf, len, MSG_NOSIGNAL);
                if (nwrote >= 0)
                {
                        // TODO
                        // if writeCompleteCallback
                }
                else
                {
                        nwrote = 0;
                        if (errno != EWOULDBLOCK)
                        {
                                // TODO LOG SYSERR
                        }
                }
        }

        assert(nwrote >= 0);

        if (static_cast<size_t>(nwrote) < len)
        {
                outputBuffer_.appen(static_cast<const char *>(buf) + nwrote, len - nwrote);
                if (!channel_->isWriting())
                {
                        channel_->enableWrite();
                }
        }
}

void TcpConnection::shutdownWrite()
{
        if (tcpConnectionState_.load(std::memory_order_relaxed) == kConnected)
        {
                setState(kDisconnecting);
                auto p = shared_from_this();
                loop_->runInLoop([p]()
                                 { p->shutdownWrite(); });
        }
}

void TcpConnection::shutdownInLoop()
{
        loop_->assertInLoopThread();
        if (channel_->isWriting())
        {
                channel_->disableWrite();
                socket_->shutdownWrite();
        }
}

void TcpConnection::forceClose()
{
        if (tcpConnectionState_.load(std::memory_order_acquire) == kConnected || tcpConnectionState_.load(std::memory_order_acquire) == kDisconnecting)
        {
                setState(kDisconnected);
                auto p = shared_from_this();
                loop_->runInLoop([p]()
                                 { p->forceCloseInLoop(); });
        }
}

void TcpConnection::forceCloseInLoop()
{
        loop_->assertInLoopThread();
        if (tcpConnectionState_.load(std::memory_order_acquire) == kConnected || tcpConnectionState_.load(std::memory_order_acquire) == kDisconnecting)
        {
                handleClose();
        }
}

void TcpConnection::setTcpNoDelay(bool on)
{
        socket_->setTcpNoDelay(on);
}

void TcpConnection::setEtTtrigger(bool on)
{
        on ? channel_->enableEt() : channel_->disableEt();
}

void TcpConnection::startRead()
{
        auto p = shared_from_this();
        loop_->runInLoop([p]()
                         { p->startReadInLoop(); });
}

void TcpConnection::startReadInLoop()
{
        if (!reading_.load(std::memory_order_relaxed) || !channel_->isReading())
        {
                channel_->enableRead();
                reading_.store(true, std::memory_order_relaxed);
        }
}

void TcpConnection::stopRead()
{
        auto p = shared_from_this();
        loop_->runInLoop([p]()
                         { p->stopReadInLoop(); });
}

void TcpConnection::stopReadInLoop()
{
        if (reading_.load(std::memory_order_relaxed) || channel_->isReading())
        {
                channel_->disableRead();
                reading_.store(false, std::memory_order_relaxed);
        }
}

void TcpConnection::connectEstablished()
{
        loop_->assertInLoopThread();
        assert(tcpConnectionState_.load(std::memory_order_relaxed) == kConnecting);
        setState(kConnected);
        channel_->enableRead();

        connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
        loop_->assertInLoopThread();
        if (tcpConnectionState_.load(std::memory_order_relaxed) == kConnected)
        {
                setState(kDisconnected);
                channel_->disableAll();

                connectionCallback_(shared_from_this());
        }
        channel_->remove();
}

void TcpConnection::handleRead()
{
        loop_->assertInLoopThread();
        int saveError = 0;
        int Error = 0;
        ssize_t n = 0;
        while (true)
        {
                n = inputBuffer_.readFdToThis(channel_->fd(), &saveError);
                Error = errno;
                if (n > 0)
                {
                        continue;
                }
                else if (n == 0 && Error == EAGAIN)
                {
                        messageCallback_(shared_from_this(), inputBuffer_);
                        handleClose();
                        break;
                }
                else
                {
                        errno = saveError;
                        handleError();
                        break;
                }
        }
}

void TcpConnection::handleWrite()
{
        loop_->assertInLoopThread();

        ssize_t n = ::send(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes(), MSG_NOSIGNAL);
        if (n > 0)
        {
                outputBuffer_.retrieve(n);
                if (outputBuffer_.readableBytes() == 0)
                {
                        if (tcpConnectionState_.load(std::memory_order_relaxed) == kDisconnecting)
                        {
                                shutdownInLoop();
                        }
                }
        }
        else
        {
                // LOG SYSERR TODO
        }
}

void TcpConnection::handleClose()
{
        assert(tcpConnectionState_.load(std::memory_order_relaxed) == kConnecting || tcpConnectionState_.load(std::memory_order_relaxed) == kConnecting);
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
        closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
        // TODO
}

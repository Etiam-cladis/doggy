#pragma once

#include "./TcpConnection.h"

using namespace doggy;
using namespace doggy::net;

TcpConnection::TcpConnection(EventLoop *loop, int sockFd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(loop),
      tcpConnectionState_(kConnecting),
      reading_(true),
      localAddr_(localAddr),
      peerAddr_(peerAddr_),
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

        socket_->setTcpNoDelay(true);
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

void TcpConnection::send(doggy::base::Buff &buff)
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

void TcpConnection::shutdownWrite()
{
}

void TcpConnection::forceClose()
{
}

void TcpConnection::startRead()
{
}

void TcpConnection::stopRead()
{
}

void TcpConnection::connectEstablished()
{
}

void TcpConnection::connectDestroyed()
{
}

void TcpConnection::handleRead()
{
}

void TcpConnection::handleWrite()
{
}

void TcpConnection::handleClose()
{
}

void TcpConnection::handleError()
{
}

void TcpConnection::sendInLoop(const std::string &message)
{
}

void TcpConnection::sendInLoop(std::string &&message)
{
}

void TcpConnection::sendInLoop(const void *buf, size_t len)
{
}

void TcpConnection::shutdownInLoop()
{
}

void TcpConnection::forceCloseInLoop()
{
}

void TcpConnection::startReadInLoop()
{
}

void TcpConnection::stopReadInLoop()
{
}
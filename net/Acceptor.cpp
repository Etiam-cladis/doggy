#pragma once

#include "./Acceptor.h"

using namespace doggy;
using namespace doggy::net;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &lisenAddr, bool reuseport)
    : loop_(loop),
      acceptSockId_(SocketOps::createNonblockingOrDie(lisenAddr.getFamily())),
      acceptChannel_(loop_, acceptSockId_.fd()),
      listenning_(false)
{
        acceptSockId_.setReuseAddr(true);
        acceptSockId_.setReusePort(reuseport);
        acceptSockId_.bindAddress(lisenAddr);
        acceptChannel_.setReadCallback([this]()
                                       { this->handleRead(); });
}

Acceptor::~Acceptor()
{
        acceptChannel_.disableAll();
        acceptChannel_.remove();
}

void Acceptor::listen()
{
        loop_->assertInLoopThread();
        listenning_.store(true, std::memory_order_relaxed);
        acceptSockId_.listen();
        acceptChannel_.enableRead();
}

void Acceptor::handleRead()
{
        loop_->assertInLoopThread();

        while (true)
        {
                InetAddress peerAddr;
                int connfd = acceptSockId_.accept(peerAddr);
                if (connfd >= 0)
                {
                        if (newConnectionCallBack_)
                        {
                                newConnectionCallBack_(connfd, peerAddr);
                        }
                        else
                        {
                                ::close(connfd);
                        }
                }
                else
                {
                        // EAGAIN
                        // EMFIFE
                        // TODO
                        break;
                }
        }
}

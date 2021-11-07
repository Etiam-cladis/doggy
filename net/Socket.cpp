#include "./Socket.h"

using namespace doggy;
using namespace doggy::net;

Socket::~Socket()
{
        SocketOps::close(sockFd_);
}

bool Socket::getTcpInfo(tcp_info *tcpInfo) const
{
        socklen_t n = sizeof(tcp_info);
        memset(tcpInfo, 0, n);
        return ::getsockopt(sockFd_, SOL_TCP, TCP_INFO, tcpInfo, &n) == 0;
}

void Socket::bindAddress(InetAddress &localaddr)
{
        SocketOps::bindOrDie(sockFd_, localaddr.getSockAddr());
}

void Socket::listen()
{
        SocketOps::listenOrDie(sockFd_);
}
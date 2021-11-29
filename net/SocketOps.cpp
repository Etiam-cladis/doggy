#pragma once

#include "./SocketOps.h"

using namespace doggy;
using namespace doggy::SocketOps;

int SocketOps::createNonblockingOrDie(sa_family_t family)
{
        int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (sockfd < 0)
        {
                // LOG SYSFATAL
                abort();
        }
        return sockfd;
}

void SocketOps::bindOrDie(int sockfd, const sockaddr *addr)
{
        int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(*addr)));
        if (ret < 0)
        {
                // LOG FATAL
                abort();
        }
}

void SocketOps::listenOrDie(int sockfd)
{
        int ret = ::listen(sockfd, SOMAXCONN);
        if (ret < 0)
        {
                // LOG FATAL
                abort();
        }
}

int SocketOps::connect(int sockfd, const sockaddr *addr)
{
        return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(*addr)));
}

int SocketOps::accept(int sockfd, sockaddr_in6 *addr)
{
        socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
        int connfd = ::accept4(sockfd, reinterpret_cast<sockaddr *>(addr),
                               &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (connfd < 0)
        {
                int savedErrno = errno;
                switch (savedErrno)
                {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO:
                case EPERM:
                case EMFILE:
                        errno = savedErrno;
                        break;
                case EBADF:
                case EFAULT:
                case EINVAL:
                case ENFILE:
                case ENOBUFS:
                case ENOMEM:
                case ENOTSOCK:
                case EOPNOTSUPP:
                        abort();
                default:
                        abort();
                }
        }
        return connfd;
}

void SocketOps::close(int sockfd)
{
        if (::close(sockfd) < 0)
        {
                // LOG SYSERR
                abort();
        }
}

void SocketOps::shutdownWrite(int sockfd)
{
        if (::shutdown(sockfd, SHUT_WR) < 0)
        {
                // LOG SYSERR
                abort();
        }
}

sockaddr_in6 SocketOps::getLocalAddr(int sockfd)
{
}

sockaddr_in6 SocketOps::getPeerAddr(int sockfd)
{
}

int SocketOps::getSocketError(int sockfd)
{
}

bool SocketOps::isSelfConnect(int sockfd)
{
}
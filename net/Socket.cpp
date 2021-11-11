#include "./Socket.h"

using namespace doggy;
using namespace doggy::net;

Socket::~Socket()
{
        SocketOps::close(sockFd_);
}

bool Socket::getTcpInfo(tcp_info *tcpInfo) const
{
        socklen_t n = sizeof(*tcpInfo);
        memset(tcpInfo, 0, n);
        return ::getsockopt(sockFd_, SOL_TCP, TCP_INFO, tcpInfo, &n) == 0;
}

void Socket::bindAddress(InetAddress &localaddr)
{
        SocketOps::bindOrDie(sockFd_, localaddr.getSockaddr());
}

void Socket::listen()
{
        SocketOps::listenOrDie(sockFd_);
}

int Socket::accept(InetAddress &peeraddr)
{
        sockaddr sa{0};
        socklen_t size = sizeof(sa);
        int connFd = ::accept4(sockFd_, &sa, &size, SOCK_CLOEXEC | SOCK_NONBLOCK);
        if (connFd < 0)
        {
                int savedErrno = errno;
                // LOG SYSERR TODO
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
                        // LOG FATAL TODO
                        break;
                default:
                        // LOG FATAL TODO
                        break;
                }
        }

        switch (sa.sa_family)
        {
        case AF_INET:
        {
                sockaddr_in *sin = reinterpret_cast<sockaddr_in *>(&sa);
                peeraddr = std::move(*sin);
                break;
        }
        case AF_INET6:
        {
                sockaddr_in6 *sin = reinterpret_cast<sockaddr_in6 *>(&sa);
                peeraddr = std::move(*sin);
                break;
        }
        default:
                abort();
        }

        return connFd;
}

int Socket::accept()
{
        int connFd = ::accept4(sockFd_, nullptr, 0, SOCK_CLOEXEC | SOCK_NONBLOCK);
        if (connFd < 0)
        {
                int savedErrno = errno;
                // LOG SYSERR TODO
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
                        // LOG FATAL TODO
                        break;
                default:
                        // LOG FATAL TODO
                        break;
                }
        }
        return connFd;
}

void Socket::shutdownWrite()
{
        SocketOps::shutdownWrite(sockFd_);
}

void Socket::setTcpNoDelay(bool on)
{
        int val = on;
        if (::setsockopt(sockFd_, IPPROTO_TCP, TCP_NODELAY, &val, static_cast<socklen_t>(sizeof(val))) < 0 && on)
        {
                // LOG SYSERR TODO
        }
}
void Socket::setReuseAddr(bool on)
{
        int val = on;
        if (::setsockopt(sockFd_, SOCK_STREAM, SO_REUSEADDR, &val, static_cast<socklen_t>(sizeof(val))) < 0 && on)
        {
                // LOG SYSERR TODO
        }
}
void Socket::setReusePort(bool on)
{
        int val = on;
        if (::setsockopt(sockFd_, SOCK_STREAM, SO_REUSEADDR, &val, static_cast<socklen_t>(sizeof(val))) < 0 && on)
        {
                // LOG SYSERR TODO
        }
}
void Socket::setKeepAlive(bool on)
{
        int val = on;
        if (::setsockopt(sockFd_, SOCK_STREAM, SO_KEEPALIVE, &val, static_cast<socklen_t>(sizeof(val))) < 0 && on)
        {
                // LOG SYSERR TODO
        }
}

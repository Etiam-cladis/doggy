#include "doggy/net/SocketsOps.h"

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
        sockaddr_in6 sa{0};
        socklen_t len = static_cast<socklen_t>(sizeof(sa));
        if (::getsockname(sockfd, reinterpret_cast<sockaddr *>(&sa), &len) < 0)
        {
                // LOG SYSERR
                abort();
        }
        return sa;
}

sockaddr_in6 SocketOps::getPeerAddr(int sockfd)
{
        sockaddr_in6 sa{0};
        socklen_t len = static_cast<socklen_t>(sizeof(sa));
        if (::getpeername(sockfd, reinterpret_cast<sockaddr *>(&sa), &len) < 0)
        {
                // LOG SYSERR
                abort();
        }
        return sa;
}

int SocketOps::getSocketError(int sockfd)
{
        int optval;
        socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        {
                return errno;
        }
        else
        {
                return optval;
        }
}

bool SocketOps::isSelfConnect(int sockfd)
{
        sockaddr_in6 localAddr = getLocalAddr(sockfd);
        sockaddr_in6 peerAddr = getPeerAddr(sockfd);
        if (localAddr.sin6_family == AF_INET)
        {
                sockaddr_in *la = reinterpret_cast<sockaddr_in *>(&localAddr);
                sockaddr_in *pa = reinterpret_cast<sockaddr_in *>(&peerAddr);
                return la->sin_port == pa->sin_port && la->sin_addr.s_addr == pa->sin_addr.s_addr;
        }
        else if (localAddr.sin6_family == AF_INET6)
        {
                return localAddr.sin6_port == peerAddr.sin6_port && std::memcmp(&localAddr.sin6_addr, &peerAddr.sin6_addr, sizeof(localAddr.sin6_addr)) == 0;
        }
        else
        {
                return false;
        }
}
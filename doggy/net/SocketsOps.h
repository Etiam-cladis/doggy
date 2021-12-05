#ifndef DOGGY_NET_SOCKETOPS_H
#define DOGGY_NET_SOCKETOPS_H

#include "doggy/net/header.h"

namespace doggy
{
        namespace SocketOps
        {

                ssize_t readv(int fd, const iovec *iov, int iovCount);

                ssize_t readv(int fd, const iovec *iov, int iovCount, off_t offset);

                ssize_t readv(int fd, const iovec *iov, int iovCount, off_t offset, int flags);

                int createNonblockingOrDie(sa_family_t family);
                int connect(int sockfd, const sockaddr *addr);
                void bindOrDie(int sockfd, const sockaddr *addr);
                void listenOrDie(int sockfd);
                int accept(int sockfd, sockaddr_in6 *addr);
                void close(int sockfd);
                void shutdownWrite(int sockfd);
                sockaddr_in6 getLocalAddr(int sockfd);
                sockaddr_in6 getPeerAddr(int sockfd);
                int getSocketError(int sockfd);
                bool isSelfConnect(int sockfd);
        } // namespace Socket

} // namespace doggy

#endif // DOGGY_NET_SOCKETOPS_H

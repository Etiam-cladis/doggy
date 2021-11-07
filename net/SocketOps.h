#pragma once

#include "./header.h"

namespace doggy
{
        namespace SocketOps
        {

                ssize_t readv(int fd, const struct iovec *iov, int iovCount)
                {
                        return ::readv(fd, iov, iovCount);
                }

                ssize_t readv(int fd, const struct iovec *iov, int iovCount, off_t offset)
                {
                        return ::preadv(fd, iov, iovCount, offset);
                }

                ssize_t readv(int fd, const struct iovec *iov, int iovCount, off_t offset, int flags)
                {
                        return ::preadv2(fd, iov, iovCount, offset, flags);
                }

                int createNonblockingOrDie(sa_family_t family);
                int connect(int sockfd, const sockaddr *addr);
                void bindOrDie(int sockfd, const sockaddr *addr);
                void listenOrDie(int sockfd);
                int accept(int sockfd, sockaddr_in6 *addr);
                void close(int sockfd);
                void shutdownWrite(int sockfd);
                sockaddr_in6 getLocalAddr(int sockfd);
                sockaddr_in6 getPeerAddr(int sockfd);
        } // namespace Socket

} // namespace doggy

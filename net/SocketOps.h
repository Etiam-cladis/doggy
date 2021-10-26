#pragma once

#include "./header.h"

namespace doggy
{
        namespace Socket
        {
                // trans sockaddr_in or sockaddr_in6 to sockaddr
                template <typename T>
                const sockaddr *sockaddr_cast(const T *socketAddress)
                {
                        return reinterpret_cast<sockaddr *>(socketAddress);
                }

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
        } // namespace Socket

} // namespace doggy

#include "doggy/net/header.h"

#include "doggy/net/Buff.h"
#include "doggy/net/SocketsOps.h"

using namespace doggy;
using namespace doggy::net;

ssize_t Buff::readFdToThis(int fd, int *saveErrno)
{
        char extraBuff[65535];
        struct iovec vec[2];
        size_t writable = writableBytes();
        vec[0].iov_base = begin() + writeIndex_;
        vec[0].iov_len = writable;
        vec[1].iov_base = extraBuff;
        vec[1].iov_len = 65535;
        ssize_t n = doggy::SocketOps::readv(fd, vec, 2);
        if (n < 0)
        {
                *saveErrno = errno;
        }
        else if (static_cast<size_t>(n) <= writable)
        {
                writeIndex_ += n;
        }
        else
        {
                writeIndex_ = buff_.size();
                appen(extraBuff, n - writable);
        }

        return n;
}

ssize_t Buff::readFdToThis(int fd, int *saveErrno, off_t offset)
{
        char extraBuff[65535];
        struct iovec vec[2];
        size_t writable = writableBytes();
        vec[0].iov_base = begin() + writeIndex_;
        vec[0].iov_len = writable;
        vec[1].iov_base = extraBuff;
        vec[1].iov_len = 65535;
        ssize_t n = doggy::SocketOps::readv(fd, vec, 2, offset);
        if (n < 0)
        {
                *saveErrno = errno;
        }
        else if (static_cast<size_t>(n) <= writable)
        {
                writeIndex_ += n;
        }
        else
        {
                writeIndex_ = buff_.size();
                appen(extraBuff, n - writable);
        }
        return n;
}

ssize_t Buff::readFdToThis(int fd, int *saveErrno, off_t offset, int flags)
{
        char extraBuff[65535];
        struct iovec vec[2];
        size_t writable = writableBytes();
        vec[0].iov_base = begin() + writeIndex_;
        vec[0].iov_len = writable;
        vec[1].iov_base = extraBuff;
        vec[1].iov_len = 65535;
        ssize_t n = doggy::SocketOps::readv(fd, vec, 2, offset, flags);
        if (n < 0)
        {
                *saveErrno = errno;
        }
        else if (static_cast<size_t>(n) <= writable)
        {
                writeIndex_ += n;
        }
        else
        {
                writeIndex_ = buff_.size();
                appen(extraBuff, n - writable);
        }
        return n;
}
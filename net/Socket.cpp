#include "./Socket.h"

using namespace doggy;
using namespace doggy::net;

Socket::~Socket()
{
        socketOps::close(sockFd_);
}

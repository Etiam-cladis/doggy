#pragma once

#include "./header.h"
#include "./Socket.h"
#include "./SocketOps.h"

namespace doggy
{
        namespace net
        {
                class InetAddress
                {
                public:
                public:
                        const sockaddr *getSockAddr();

                private:
                        std::variant<sockaddr_in, sockaddr_in6> sockaddr_;
                };

        } // namespaace net
} // namespace doggy

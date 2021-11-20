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
                        InetAddress() = default;
                        explicit InetAddress(uint16_t port, bool loopback = false, bool ipv6 = false);

                        // ip shoule be "1.2.3.4"
                        InetAddress(const std::string &ip, uint16_t port, bool ipv6 = false);

                        explicit InetAddress(const sockaddr_in &sa) : sockaddr_(sa) {}
                        explicit InetAddress(sockaddr_in &&sa) noexcept : sockaddr_(std::move(sa)) {}
                        explicit InetAddress(const sockaddr_in6 &sa) : sockaddr_(sa) {}
                        explicit InetAddress(sockaddr_in6 &&sa) noexcept : sockaddr_(std::move(sa)) {}

                        explicit InetAddress(const InetAddress &) = default;

                        InetAddress &operator=(sockaddr_in &&sa) { sockaddr_ = std::move(sa); }

                        InetAddress &operator=(sockaddr_in6 &&sa) { sockaddr_ = std::move(sa); }

                public:
                        const sockaddr *getSockaddr() const;
                        sa_family_t getFamily() const;
                        std::string inetNtop();

                private:
                        std::variant<sockaddr_in, sockaddr_in6> sockaddr_;
                };

        } // namespaace net
} // namespace doggy

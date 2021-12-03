#include "doggy/net/InetAddress.h"

#include "doggy/net/SocketsOps.h"

using namespace doggy;
using namespace doggy::net;

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
        if (ipv6)
        {
                sockaddr_ = sockaddr_in6{0};
                auto sa = std::get_if<sockaddr_in6>(&sockaddr_);
                sa->sin6_family = AF_INET6;
                in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
                sa->sin6_addr = ip;
                sa->sin6_port = ::htons(port);
        }
        else
        {
                sockaddr_ = sockaddr_in{0};
                auto sa = std::get_if<sockaddr_in>(&sockaddr_);
                sa->sin_family = AF_INET;
                in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
                sa->sin_addr.s_addr = ip;
                sa->sin_port = ::htons(port);
        }
}

InetAddress::InetAddress(const std::string &ip, uint16_t port, bool ipv6)
{
        if (ipv6)
        {
                sockaddr_ = sockaddr_in6{0};
                auto sa = std::get_if<sockaddr_in6>(&sockaddr_);
                sa->sin6_family = AF_INET6;
                sa->sin6_port = ::htons(port);
                if (::inet_pton(AF_INET6, ip.c_str(), &sa->sin6_addr) <= 0)
                {
                        // LOG SYSERR
                }
        }
        else
        {
                sockaddr_ = sockaddr_in6{0};
                auto sa = std::get_if<sockaddr_in>(&sockaddr_);
                sa->sin_family = AF_INET;
                sa->sin_port = ::htons(port);
                if (::inet_pton(AF_INET6, ip.c_str(), &sa->sin_addr) <= 0)
                {
                        // LOG SYSERR
                }
        }
}

const sockaddr *InetAddress::getSockaddr() const
{
        return std::visit([](auto &&sa) -> const sockaddr *
                          {
                                                          using T = std::decay_t<decltype(sa)>;
                                                          if (std::is_same_v<T, sockaddr_in> || std::is_same_v<T, sockaddr_in6>)
                                                          {
                                                                  return reinterpret_cast<const sockaddr *>(&sa);
                                                          }
                                                          else
                                                          {
                                                                  // LOG FATAL TODO
                                                                  abort();
                                                          } },
                          sockaddr_);
}

sa_family_t InetAddress::getFamily() const
{
        int index = static_cast<int>(sockaddr_.index());
        switch (index)
        {
        case 0:
                return std::get<0>(sockaddr_).sin_family;
        case 1:
                return std::get<1>(sockaddr_).sin6_family;
        default:
                // LOG FATAL TODO
                abort();
        }

        return 0;
}

std::string InetAddress::inetNtop()
{
        char buf[64] = "";
        int size = sizeof(buf);
        int index = static_cast<int>(sockaddr_.index());
        if (index == 0)
        {
                assert(size >= INET_ADDRSTRLEN);
                auto addr = std::get_if<0>(&sockaddr_);
                ::inet_ntop(AF_INET, &addr->sin_addr, buf, static_cast<socklen_t>(size));
        }
        else if (index == 1)
        {
                assert(size >= INET6_ADDRSTRLEN);
                auto addr = std::get_if<1>(&sockaddr_);
                ::inet_ntop(AF_INET6, &addr->sin6_addr, buf, static_cast<socklen_t>(size));
        }
        return buf;
}

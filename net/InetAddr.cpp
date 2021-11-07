#pragma once

#include "./InetAddr.h"

using namespace doggy;
using namespace net;

const sockaddr *InetAddress::getSockAddr()
{
        return std::visit([](auto &&SA) -> const sockaddr *
                          {
                                                          using T = std::decay_t<decltype(SA)>;
                                                          if (std::is_same_v<T, sockaddr_in> || std::is_same_v<T, sockaddr_in6>)
                                                          {
                                                                  return reinterpret_cast<sockaddr *>(&SA);
                                                          }
                                                          else
                                                          {
                                                                  // LOG FATAL TODO
                                                                  abort();
                                                          } },
                          sockaddr_);
}

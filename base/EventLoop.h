#pragma once

#include "./header.h"

#include "../net/Channel.h"

namespace doggy
{
        namespace base
        {
                class EventLoop
                {
                public:
                        void updateChannel(doggy::net::Channel *channel);
                        void removeChannel(doggy::net::Channel *channel);

                private:
                };

        } // namespace base

} // namespace doggy
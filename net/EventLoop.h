#pragma once

#include "./header.h"

#include "../net/Channel.h"

namespace doggy
{
        namespace net
        {
                class EventLoop
                {
                public:
                        void updateChannel(Channel *channel);
                        void removeChannel(Channel *channel);

                private:
                };

        } // namespace net

} // namespace doggy
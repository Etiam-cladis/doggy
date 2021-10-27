#pragma once

#include "../base/EventLoop.h"

namespace doggy
{
        namespace net
        {

                class Channel
                {
                        using EventCallback = std::function<void()>;

                private:
                        static constexpr int kNodeEvent = 0;
                        static constexpr int kReadEvent_ = EPOLLIN | EPOLLPRI;
                        static constexpr int kwriteEvent_ = EPOLLOUT;

                        doggy::base::EventLoop *loop_;
                        int fd_;

                        EventCallback writeCallback_;
                        EventCallback readCallback_;
                        EventCallback closeCallback_;
                };
        } // namespace base

} // namespace doggy
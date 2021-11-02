#pragma once

#include "./EventLoop.h"

namespace doggy
{
        namespace net
        {
                class Channel
                {
                        using EventCallback = std::function<void()>;

                public:
                        Channel(EventLoop *loop, int fd);
                        ~Channel();

                public:
                        Channel(const Channel &) = delete;
                        Channel &operator=(const Channel &) = delete;

                public:
                        int fd() const { return fd_; }

                        int index() const { return index_; }

                        void setIndex(int inx) { index_ = inx; }

                        int events() const { return event_; }

                        void setRevents(int rev) { rEvent_ = rev; }

                        bool isNoneEvent() const { return event_ == kNoneEvent; }

                        void enableRead()
                        {
                                event_ |= kReadEvent_;
                                update();
                        }

                        void disableRead()
                        {
                                event_ &= ~kReadEvent_;
                                update();
                        }

                        void enableWrite()
                        {
                                event_ |= kWriteEvent_;
                                update();
                        }

                        void disableWrite()
                        {
                                event_ &= ~kWriteEvent_;
                                update();
                        }

                        void enableRdhup()
                        {
                                event_ |= kRdhup_;
                                update();
                        }

                        void disableRdhup()
                        {
                                event_ &= ~kRdhup_;
                                update();
                        }

                        void enableOneshot()
                        {
                                event_ |= kOneshot_;
                                update();
                        }

                        void disableOneshot()
                        {
                                event_ &= ~kOneshot_;
                                update();
                        }

                        void enableEt()
                        {
                                event_ |= kEt_;
                                update();
                        }

                        void disableEt()
                        {
                                event_ &= ~kEt_;
                                update();
                        }

                        void disableAll()
                        {
                                event_ = kNoneEvent;
                                update();
                        }

                        bool isReading() const { return event_ & kReadEvent_; }

                        bool isWriting() const { return event_ & kWriteEvent_; }

                        bool isRdhup() const { return event_ & kRdhup_; }

                        bool isOneshot() const { return event_ & kOneshot_; }

                        bool isEt() const { return event_ & kEt_; }

                        void setReadCallback(const EventCallback &cb) { readCallback_ = cb; }

                        void setReadCallback(EventCallback &&cb) { readCallback_ = std::move(cb); }

                        void setWriteCallback(const EventCallback &cb) { writeCallback_ = cb; }

                        void setWriteCallback(EventCallback &&cb) { writeCallback_ = std::move(cb); }

                        void setCloseCallback(const EventCallback &cb) { closeCallback_ = cb; }

                        void setCloseCallback(EventCallback &&cb) { closeCallback_ = std::move(cb); }

                        void setErrorCallback(const EventCallback &cb) { errorCallback_ = cb; }

                        void setErrorCallback(EventCallback &&cb) { errorCallback_ = std::move(cb); }

                public:
                        void remove();
                        void handleEvent();
                        EventLoop *ownerLoop() const { return loop_; }

                private:
                        void update();

                        static constexpr int kNoneEvent = 0;
                        static constexpr int kReadEvent_ = EPOLLIN | EPOLLPRI;
                        static constexpr int kWriteEvent_ = EPOLLOUT;
                        static constexpr int kRdhup_ = EPOLLRDHUP;
                        static constexpr int kOneshot_ = EPOLLONESHOT;
                        static constexpr int kEt_ = EPOLLET;

                        EventLoop *loop_;
                        int fd_;
                        int event_;

                        int rEvent_; // for epoll
                        int index_;  // for epoll

                        bool eventHading_;
                        bool addedToLoop_;

                        EventCallback writeCallback_;
                        EventCallback readCallback_;
                        EventCallback closeCallback_;
                        EventCallback errorCallback_;
                };
        } // namespace net

} // namespace doggy
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

                        bool isNoneRWEvent() const { return !isReading() && !isWriting(); }

                        void enableRead()
                        {
                                event_ |= kReadEvent_;
                                updateRW();
                        }

                        void disableRead()
                        {
                                event_ &= ~kReadEvent_;
                                updateRW();
                        }

                        void enableWrite()
                        {
                                event_ |= kWriteEvent_;
                                updateRW();
                        }

                        void disableWrite()
                        {
                                event_ &= ~kWriteEvent_;
                                updateRW();
                        }

                        void enableRdhup()
                        {
                                event_ |= kRdhup_;
                                updateOther();
                        }

                        void disableRdhup()
                        {
                                event_ &= ~kRdhup_;
                                updateOther();
                        }

                        void enableOneshot()
                        {
                                event_ |= kOneshot_;
                                updateOther();
                        }

                        void disableOneshot()
                        {
                                event_ &= ~kOneshot_;
                                updateOther();
                        }
                        void enableEt()
                        {
                                event_ |= kEt_;
                                updateOther();
                        }
                        void disableEt()
                        {
                                event_ &= ~kEt_;
                                updateOther();
                        }
                        void disableAll()
                        {
                                event_ = 0;
                                updateRW();
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
                        void updateRW();
                        void updateOther();

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
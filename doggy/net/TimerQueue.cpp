#include "doggy/net/TimerQueue.h"

#include "doggy/net/EventLoop.h"
#include "doggy/net/Timer.h"
#include "doggy/net/TimerId.h"

using namespace doggy;
using namespace doggy::net;

using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;
using Entry = std::pair<Timestamp, std::unique_ptr<Timer>>;
using TimerList = std::set<Entry, std::less<>>;
using ActiveTimer = std::pair<std::unique_ptr<Timer>, int64_t>;
using ActiveTimerSet = std::set<ActiveTimer, std::less<>>;

namespace
{
        int creatTimerFd()
        {
                int timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
                if (timerFd < 0)
                {
                        abort();
                }
                return timerFd;
        }

        timespec howMuchTimeFromNow(Timestamp when)
        {
                using namespace std::chrono;
                auto now = time_point_cast<microseconds>(system_clock::now());
                auto diff = when - now;
                if (diff < 100us)
                {
                        diff = 100us;
                }

                timespec ts{0};
                ts.tv_sec = static_cast<time_t>(diff.count() / (1000 * 1000));
                ts.tv_nsec = static_cast<long>((diff.count() % (1000 * 1000)) * 1000);
                return ts;
        }

        void readTimerFd(int timerFd)
        {
                uint64_t howmany;
                ssize_t n = ::read(timerFd, &howmany, sizeof(howmany));
                if (n != sizeof(howmany))
                {
                        abort();
                }
        }

        void resetTimerFd(int timerFd, Timestamp expiration)
        {
                itimerspec newValue{0};
                itimerspec oldValue{0};
                newValue.it_value = howMuchTimeFromNow(expiration);
                if (::timerfd_settime(timerFd, 0, &newValue, &oldValue))
                {
                        abort();
                }
        }
}

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerFd_(creatTimerFd()),
      timerFdChannel_(loop, timerFd_),
      timers_(),
      callingExpiredTimers_(false)
{
        timerFdChannel_.setReadCallback([this]()
                                        { this->handleRead(); });
}

TimerQueue::~TimerQueue()
{
        timerFdChannel_.disableAll();
        timerFdChannel_.remove();
        ::close(timerFd_);
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, std::chrono::microseconds interval)
{
#ifndef USE_LT_TRIGGER
        if (!timerFdChannel_.isEt())
        {
                timerFdChannel_.enableEt();
        }
#endif

        if (!timerFdChannel_.isReading())
        {
                timerFdChannel_.enableRead();
        }

        std::shared_ptr<Timer> timer = std::make_shared<Timer>(cb, when, interval);
        auto p = timer.get();
        loop_->runInLoop([timer = std::move(timer), this]() mutable
                         { this->addTimerInLoop(timer); });
        return TimerId(p, p->sequence());
}
TimerId TimerQueue::addTimer(TimerCallback &&cb, Timestamp when, std::chrono::microseconds interval)
{
#ifndef USELTTRIGGER
        if (!timerFdChannel_.isEt())
        {
                timerFdChannel_.enableEt();
        }
#endif

        if (!timerFdChannel_.isReading())
        {
                timerFdChannel_.enableRead();
        }

        std::shared_ptr<Timer> timer = std::make_shared<Timer>(std::move(cb), when, interval);
        auto p = timer.get();
        loop_->runInLoop([timer = std::move(timer), this]() mutable
                         { this->addTimerInLoop(timer); });
        return TimerId(p, p->sequence());
}

void TimerQueue::addTimerInLoop(std::shared_ptr<Timer> &timer)
{
        loop_->assertInLoopThread();
        auto p = timer.get();
        bool earliestChanged = insert(timer);

        if (earliestChanged)
        {
                resetTimerFd(timerFd_, p->expiration());
        }
}

void TimerQueue::cancel(TimerId timerId)
{
        loop_->runInLoop([this, &timerId]()
                         { this->cancelInLoop(timerId); });
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
        loop_->assertInLoopThread();
        assert(timers_.size() == activeTimers_.size());
        ActiveTimer timer(timerId.timer_, timerId.sequence_);
        ActiveTimerSet::iterator it = activeTimers_.find(timer);
        if (it != activeTimers_.end())
        {
                auto iter = timers_.find(Entry(it->first->expiration(), it->first));
                timers_.erase(iter);
                activeTimers_.erase(it);
        }
        else if (callingExpiredTimers_)
        {
                cancelingTimers_.insert(timer);
        }
        assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead()
{

        loop_->assertInLoopThread();
        Timestamp now(std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()));
        readTimerFd(timerFd_);

        std::vector<Entry> expired;
        getExpired(expired, now);

        callingExpiredTimers_.store(true, std::memory_order_relaxed);
        cancelingTimers_.clear();

        for (const Entry &it : expired)
        {
                it.second->run();
        }
        callingExpiredTimers_.store(false, std::memory_order_relaxed);

        reset(expired, now);
}

void TimerQueue::getExpired(std::vector<Entry> &expired, Timestamp now)
{
        assert(timers_.size() == activeTimers_.size());
        auto end = timers_.lower_bound({now, nullptr});
        assert(end == timers_.end() || end->first > now);

        for (auto begin = timers_.begin(); begin != end;)
        {
                size_t n = activeTimers_.erase({(*begin).second.get(), (*begin).second->sequence()});
                assert(n == 1);
                auto expir = timers_.extract(begin++);
                expired.emplace_back(std::move(expir.value()));
        }

        assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::reset(std::vector<Entry> &expired, Timestamp now)
{
        Timestamp nextExpire;

        for (Entry &it : expired)
        {
                if (it.second->repeat() && cancelingTimers_.find({it.second.get(), it.second->sequence()}) == cancelingTimers_.end())
                {
                        it.second->restart(now);
                        insert(it.second);
                }
        }

        if (!timers_.empty())
        {
                nextExpire = timers_.begin()->second->expiration();
        }

        if (nextExpire.time_since_epoch().count())
        {
                resetTimerFd(timerFd_, nextExpire);
        }
}

bool TimerQueue::insert(std::shared_ptr<Timer> &timer)
{
        loop_->assertInLoopThread();
        assert(timers_.size() == activeTimers_.size());
        bool earliestChanged = false;
        Timestamp when = timer->expiration();
        auto it = timers_.begin();
        if (it == timers_.end() || when < it->first)
        {
                earliestChanged = true;
        }

        auto p = timer.get();
        auto &&[iterator, inserted] = activeTimers_.emplace(std::move(ActiveTimer{p, p->sequence()}));
        assert(inserted);

        auto &&[iter, i] = timers_.emplace(std::move(Entry{when, std::move(timer)}));
        assert(i);

        assert(timers_.size() == activeTimers_.size());
        return earliestChanged;
}
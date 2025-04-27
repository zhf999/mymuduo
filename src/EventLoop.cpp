//
// Created by zhouhf on 2025/4/12.
//


#include <sys/eventfd.h>
#include <memory>
#include "EventLoop.h"
#include "Logger.h"
#include "PollerFactory.h"
#include "Channel.h"

static int createEventFd()
{
    int evtFd = eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
    if(evtFd < 0)
    {
        mymuduo::Logger::LogFatal("eventfd error:{}",errno);
    }
    return evtFd;
}

namespace mymuduo {
    // 单例模式
    thread_local EventLoop *t_loopInThisThread = nullptr;
    const int kPollTimeoutMs = 10000;

    // 创建wakeup_fd,用于唤醒subLoop

    EventLoop::EventLoop()
        :threadId_(CurrentThread::tid()),
        poller_(PollerFactory::newDefaultPoller(this)),
        wakeupChannel_(new Channel(this,createEventFd()))
    {
        Logger::LogInfo("EventLoop::constructor - EventLoop created {} in thread {}",
                         static_cast<void*>(this),threadId_);
        if(t_loopInThisThread)
        {
            Logger::LogFatal("Another EventLoop {} exists in this thread {}",
                             static_cast<void*>(t_loopInThisThread), threadId_);
        }
        else{
            t_loopInThisThread = this;
        }

        wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));

        wakeupChannel_->enableReading();
    }

    EventLoop::~EventLoop() {
        Logger::LogDebug("EventLoop::destructor - EventLoop {} in {}",
                         static_cast<void*>(this), threadId_);
        wakeupChannel_->disableAll();
        wakeupChannel_->removeFromLoop();
        close(wakeupChannel_->fd());
        t_loopInThisThread = nullptr;
    }

    void EventLoop::handleRead() {
        Logger::LogDebug("EventLoop::handleRead - EventLoop {}",static_cast<void*>(this));
        uint64_t one = 1;
        ssize_t n = read(wakeupChannel_->fd(), &one, sizeof(one));
        if(n != sizeof(one))
        {
            Logger::LogError("EventLoop::handleRead() reads {} bytes instead of {}",
                             n, sizeof(one));
        }
    }

    void EventLoop::loop() {
        quit_ = false;
        Logger::LogInfo("EventLoop {} start looping",static_cast<void*>(this));

        while(!quit_)
        {
            activeChannels_.clear();
            // 监听两种fd，一种是client的fd，一种是wakeup的fd
            pollReturnTime_ = poller_->poll(kPollTimeoutMs, &activeChannels_);
            for(Channel *channel : activeChannels_)
            {
                channel->handleEvent(pollReturnTime_);
            }
            // 被唤醒的子Loop执行当前事件循环需要处理的回调操作
            doPendingFunctors();
        }

        Logger::LogInfo("EventLoop {} stop looping",static_cast<void*>(this));
    }

    void EventLoop::quit() {
        quit_ = true;
        if(!isInLoopThread()) // 如果是在其他线程中调用quit
        {
            wakeup();
        }
    }

    void EventLoop::runInLoop(EventLoop::Functor cb) {
        if(isInLoopThread())
        {
            cb();
        }else
        {
            queueInLoop(std::move(cb));
        }
    }

    void EventLoop::queueInLoop(EventLoop::Functor cb) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            pendingFunctors_.emplace_back(std::move(cb));
        }

        if(!isInLoopThread() || callingPendingFunctors_)
        {
            wakeup();
        }
    }

    void EventLoop::wakeup() {
        uint64_t one=1;
        ssize_t n = write(wakeupChannel_->fd(),&one,sizeof(one));
        if(n!=sizeof(one))
        {
            Logger::LogError("EventLoop::wakeup() write {} bytes instead of {}",n, sizeof(one));
        }
    }

    void EventLoop::updateChannel(Channel *channel) {
        poller_->updateChannel(channel);
    }

    void EventLoop::removeChannel(Channel *channel) {
        poller_->removeChannel(channel);
    }

    bool EventLoop::hasChannel(Channel *channel) {
        return poller_->hasChannel(channel);
    }

    void EventLoop::doPendingFunctors() {
        std::vector<Functor> functors;
        callingPendingFunctors_ = true;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            functors.swap(pendingFunctors_);
        }

        for(const Functor &functor:functors)
        {
            functor();
        }

        callingPendingFunctors_ = false;
    }


} // mymuduo

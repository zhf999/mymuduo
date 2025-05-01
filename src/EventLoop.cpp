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
        LOG_FATAL("error=%d",errno);
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
        LOG_INFO("EventLoop %p created", this);
        if(t_loopInThisThread)
        {
            LOG_FATAL("Another EventLoop %p exists", static_cast<void*>(t_loopInThisThread));
        }
        else{
            t_loopInThisThread = this;
        }

        wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));

        wakeupChannel_->enableReading();
    }

    EventLoop::~EventLoop() {
        LOG_DEBUG("Event loop=%p is destructed", this);
        wakeupChannel_->disableAll();
        wakeupChannel_->removeFromLoop();
        close(wakeupChannel_->fd());
        t_loopInThisThread = nullptr;
    }

    void EventLoop::handleRead() {
        LOG_INFO("Event loop=%p",this);
        uint64_t one = 1;
        ssize_t n = read(wakeupChannel_->fd(), &one, sizeof(one));
        if(n != sizeof(one))
        {
            LOG_ERROR("reads %d bytes instead of %d",
                             n, sizeof(one));
        }
    }

    void EventLoop::loop() {
        quit_ = false;
        LOG_INFO("EventLoop %p start looping",this);

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

        LOG_INFO("EventLoop %d stop looping", this);
    }

    void EventLoop::quit() {
        LOG_DEBUG("EventLoop %p quit", this);
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
        LOG_DEBUG("EventLoop %p wakeup", this);
        uint64_t one=1;
        ssize_t n = write(wakeupChannel_->fd(),&one,sizeof(one));
        if(n!=sizeof(one))
        {
            LOG_ERROR("EventLoop::wakeup() write %d bytes instead of %d",n, sizeof(one));
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

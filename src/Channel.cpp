//
// Created by zhouhf on 2025/4/12.
//

#include <sys/epoll.h>
#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

namespace mymuduo {
    const int Channel::kNoneEvent = 0;
    const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
    const int Channel::kWriteEvent = EPOLLOUT;

    Channel::Channel(EventLoop *loop, int fd)
        :loop_(loop),
        fd_(fd),
        events_(0),
        r_events_(0),
        index_(-1),
        is_tied_(false){
        loop_->updateChannel(this);
    }

    Channel::~Channel() = default;

    void Channel::tie(const std::shared_ptr<void> &obj) {
        tie_ = obj;
        is_tied_ = true;
    }

    void Channel::update() {
        loop_->updateChannel(this);
    }

    void Channel::removeFromLoop() {
        loop_->removeChannel(this);
    }

    void Channel::handleEvent(Timestamp receiveTime) {
        if(is_tied_)
        {
            std::shared_ptr<void> guard = tie_.lock();
            if(guard)
            {
                handleEventWithGuard(receiveTime);
            }
        }else
        {
            handleEventWithGuard(receiveTime);
        }
    }

    void Channel::handleEventWithGuard(Timestamp receiveTime) {
        Logger::LogInfo("channel handle event: {}",r_events_);
        if((r_events_ & EPOLLHUP) && !(r_events_ & EPOLLIN))
        {
            if(closeCallback_)
            {
                closeCallback_();
            }
        }

        if((r_events_ & EPOLLERR))
        {
            if(errorCallback_)
            {
                errorCallback_();
            }
        }

        if( r_events_ & (EPOLLIN| EPOLLPRI))
        {
            if(readEventCallback_)
            {
                readEventCallback_(receiveTime);
            }
        }

        if( r_events_ & EPOLLOUT)
        {
            if(writeCallback_)
            {
                writeCallback_();
            }
        }
    }

} // mymuduo
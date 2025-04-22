//
// Created by zhouhf on 2025/4/18.
//

#include <format>
#include <memory>
#include <utility>
#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

namespace mymuduo{

    EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, std::string nameArg)
    : baseLoop_{baseLoop},
    name_{std::move(nameArg)}
    {

    }

    EventLoopThreadPool::~EventLoopThreadPool() = default;

    void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb) {
        started_ = true;

        for(int i=0;i<numThreads_;++i)
        {
            std::string threadName = std::format("{}:{}",
                                                 name_,i);
            EventLoopThread *t = new EventLoopThread(cb,threadName);
            threads_.push_back(std::unique_ptr<EventLoopThread>(t));
            loops_.push_back(t->startLoop());
        }

        if(numThreads_ == 0 && cb)
        {
            cb(baseLoop_);
        }
    }

    EventLoop *EventLoopThreadPool::getNextLoop() {
        EventLoop *loop = baseLoop_;
        if(!loops_.empty())
        {
            loop = loops_[next_];
            next_++;
            if(next_>=loops_.size())
                next_ = 0;
        }
        return loop;
    }

    std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
        if(loops_.empty())
        {
            return std::vector<EventLoop*>(1,baseLoop_);
        }else
        {
            return loops_;
        }
    }


}
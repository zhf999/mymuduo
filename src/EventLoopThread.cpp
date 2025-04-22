//
// Created by zhouhf on 2025/4/18.
//

#include "EventLoopThread.h"

#include <utility>
#include "EventLoop.h"

namespace mymuduo {

    EventLoopThread::EventLoopThread(EventLoopThread::ThreadInitCallback cb, const std::string &name)
    : thread_([&](){threadFunc();},name),
    callback_(std::move(cb)){

    }

    EventLoopThread::~EventLoopThread() {
        exiting_ = true;
        if(loop_!= nullptr)
        {
            loop_->quit();
            thread_.join();
        }
    }

    EventLoop *EventLoopThread::startLoop() {
        thread_.start(); // 开启新线程执行threadFunc

        EventLoop *loop = nullptr;
        {
            std::unique_lock lock(mutex_);
            while(loop_!= nullptr)
            {
                cond_.wait(lock);
            }
            loop = loop_;
        }
        return loop;
    }

    void EventLoopThread::threadFunc() {
        // TODO: this should be allocated from heap?
        EventLoop loop;
        if(callback_)
        {
            callback_(loop_);
        }

        {
            std::unique_lock lock(mutex_);
            loop_ = &loop;
            cond_.notify_one();
        }

        // 在新线程中循环
        loop.loop();
        std::unique_lock lock(mutex_);
        loop_ = nullptr;
    }
} // mymuduo
//
// Created by zhouhf on 2025/4/18.
//
#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>

#include "noncopyable.h"
#include "Thread.h"

namespace mymuduo {
    class EventLoop;

    class EventLoopThread : noncopyable {
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;

        explicit EventLoopThread(ThreadInitCallback cb = ThreadInitCallback(),
                        const std::string &name = std::string());
        ~EventLoopThread();

        // 开启新线程执行threadFunc
        EventLoop* startLoop();
    private:
        // 在新线程中能运行
        void threadFunc();

        EventLoop *loop_{};
        bool exiting_{false};
        Thread thread_;
        std::mutex mutex_{};
        std::condition_variable cond_{};
        ThreadInitCallback callback_;
    };

} // mymuduo



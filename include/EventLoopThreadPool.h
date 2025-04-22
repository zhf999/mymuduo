//
// Created by zhouhf on 2025/4/18.
//
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "noncopyable.h"

namespace mymuduo{
    class EventLoop;
    class EventLoopThread;

    class EventLoopThreadPool:noncopyable {
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;

        EventLoopThreadPool(EventLoop *baseLoop, std::string nameArg);
        ~EventLoopThreadPool();

        void setThreadNum(int numThreads) { numThreads_ = numThreads; };

        void start(const ThreadInitCallback &cb = ThreadInitCallback());

        EventLoop* getNextLoop();
        std::vector<EventLoop*> getAllLoops();

        bool started() const { return started_; };
        std::string name() { return name_; };
    private:
        EventLoop *baseLoop_;
        std::string name_;
        bool started_{false};
        int numThreads_{0};
        int next_{0}; // 用于在loops_中轮询
        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop*> loops_;
    };

}

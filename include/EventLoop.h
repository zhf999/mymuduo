//
// Created by zhouhf on 2025/4/12.
//
#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include "noncopyable.h"
//#include "Channel.h"
#include "Timestamp.h"
//#include "Poller.h"
#include "CurrentThread.h"

namespace mymuduo {
    class Poller;
    class Channel;

    class EventLoop : noncopyable{
    public:
        using Functor = std::function<void()>;

        EventLoop();
        ~EventLoop();

        void loop();
        void quit();

        Timestamp pollReturnTime() const { return pollReturnTime_; }

        // 在当前loop中执行cb
        void runInLoop(Functor cb);
        // 把cb放入队列中，唤醒loop所在的线程，执行cb
        void queueInLoop(Functor cb);

        // 唤醒loop所在的线程
        void wakeup();

        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);

        // 判断EventLoop是否在自己的线程里面
        bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    private:
        void handleRead();
        void doPendingFunctors();

        using ChannelList = std::vector<Channel*>;

//        std::atomic_bool looping_{false};
        std::atomic_bool quit_{false}; // 标识是否退出loop循环

        const pid_t threadId_;

        Timestamp pollReturnTime_;
        std::unique_ptr<Poller> poller_;

        /*
         * 当mainLoop获取一个新用户的Channel，通过轮询算法获得一个subLoop
         * 通过wakeupFd来唤醒该线程
         */
//        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;

        ChannelList activeChannels_{};

        std::mutex mutex_; // 保存回调操作的互斥锁
        std::atomic_bool callingPendingFunctors_{false}; // 当前线程是否有需要执行的回调操作
        std::vector<Functor> pendingFunctors_; // 存储所有需要执行的回调操作
    };

} // mymuduo

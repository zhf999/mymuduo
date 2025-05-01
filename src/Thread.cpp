//
// Created by zhouhf on 2025/4/17.
//

#include "Thread.h"
#include "CurrentThread.h"
#include "Logger.h"
#include <format>
#include <memory>
#include <semaphore>
#include <utility>


namespace mymuduo {
    std::atomic_int Thread::numCreated_ {0};

    Thread::Thread(Thread::ThreadFunc func, std::string name)
    : func_(std::move(func)),
    name_(std::move(name)){
        setDefaultName();
    }

    Thread::~Thread() {
        if(started_ && !joined_)
        {
            thread_->detach();
        }
    }

    void Thread::setDefaultName() {
        int num = ++numCreated_;
        if(name_.empty())
        {
            name_ = std::format("Thread {}",num+1);
        }
    }

    void Thread::start() {
        started_ = true;
        sem_t sem;
        sem_init(&sem,false, 0);
        thread_ = std::make_shared<std::thread>([&](){
            tid_ = CurrentThread::tid();
            sem_post(&sem);
            LOG_INFO("thread %s tid=%d started...",name_.c_str(), tid_);
            func_();
        });

        sem_wait(&sem); // return after thread is started
    }

    void Thread::join() {
        joined_ = true;
        thread_->join();
    }


} // mymuduo
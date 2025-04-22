//
// Created by zhouhf on 2025/4/17.
//

#pragma once
#include <functional>
#include <thread>
#include <memory>
#include <string>
#include "noncopyable.h"

namespace mymuduo {

    class Thread : noncopyable{
    public:
        using ThreadFunc = std::function<void()>;

        explicit Thread(ThreadFunc func, std::string name=std::string());
        ~Thread();

        void start();
        void join();

        bool started() const { return started_; };
        pid_t tid() const { return tid_; };
        const std::string& name() const { return name_; };
        static int numCreated() { return numCreated_; }
    private:
        void setDefaultName();

        bool started_{false};
        bool joined_{false};
        std::shared_ptr<std::thread> thread_{nullptr};
        pid_t tid_{0};
        ThreadFunc  func_;
        std::string name_;
        static std::atomic_int numCreated_;
    };

} // mymuduo


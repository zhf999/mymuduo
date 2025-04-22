//
// Created by zhouhf on 2025/4/11.
//
#pragma once
namespace mymuduo{
    class noncopyable{
    public:
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };
}

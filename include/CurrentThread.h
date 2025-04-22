//
// Created by zhouhf on 2025/4/17.
//

#pragma once
#include <unistd.h>

namespace mymuduo::CurrentThread {
    extern thread_local int t_cachedTid;

    void cacheTid();

    inline int tid() {
        if (t_cachedTid == 0)[[unlikely]] {
            cacheTid();
        }
        return t_cachedTid;
    }
}


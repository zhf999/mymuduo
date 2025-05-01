//
// Created by zhouhf on 2025/5/1.
//

#include "CurrentThread.h"

namespace mymuduo::CurrentThread {
    thread_local int t_cachedTid = 0;

    void cacheTid()
    {
        if(t_cachedTid==0)
        {
            t_cachedTid = static_cast<pid_t>(gettid());
        }
    }

    int tid() {
        if (t_cachedTid == 0)[[unlikely]] {
            cacheTid();
        }
        return t_cachedTid;
    }
}
//
// Created by zhouhf on 2025/4/12.
//

#pragma once

#include "Poller.h"

namespace mymuduo
{
    class PollerFactory {
    public:
        static Poller* newDefaultPoller(EventLoop *loop);
    };
}



//
// Created by zhouhf on 2025/4/12.
//

#include "PollerFactory.h"
#include "EpollPoller.h"

namespace mymuduo {
    Poller *PollerFactory::newDefaultPoller(EventLoop *loop) {
        if (::getenv("MUDUO_USER_POLL")) {
            return nullptr;
        } else {
            return new EpollPoller(loop);
        }
    }
}
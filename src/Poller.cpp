//
// Created by zhouhf on 2025/4/12.
//


#include "Poller.h"
#include "Channel.h"

namespace mymuduo {
    Poller::Poller(EventLoop *loop)
        : ownerLoop(loop){

    }

    bool Poller::hasChannel(Channel *channel) {
        return channels_.contains(channel->fd());
    }

} // mymuduo
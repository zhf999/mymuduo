//
// Created by zhouhf on 2025/4/15.
//

#pragma once

#include <vector>
#include <sys/epoll.h>
#include "Poller.h"

namespace mymuduo{
    class EpollPoller: public Poller{
    public:
        explicit EpollPoller(EventLoop *loop);
        ~EpollPoller() override;

        Timestamp poll(int timeoutMs, mymuduo::Poller::ChannelList *activeChannels) override;
        bool hasChannel(mymuduo::Channel *channel) override;
        void removeChannel(mymuduo::Channel *channel) override;
        void updateChannel(mymuduo::Channel *channel) override;

    private:
        static const int kInitEventListSize = 16;
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
        void update(int operation, Channel* channel) const;

        using EventList = std::vector<epoll_event>;
        int epoll_fd_;
        EventList events_;
    };
}

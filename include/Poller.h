//
// Created by zhouhf on 2025/4/12.
//
#pragma once

#include <vector>
#include <unordered_map>
#include "noncopyable.h"
#include "Timestamp.h"

namespace mymuduo {
    class Channel;
    class EventLoop;

    class Poller:noncopyable {
    public:
        using ChannelList = std::vector<Channel*>;

        explicit Poller(EventLoop *loop);
        virtual ~Poller() = default;

        virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
        // 更新channel关心的事件（不会将channel从poller中移除）
        virtual void updateChannel(Channel *channel) = 0;
        // 从poller中彻底移除channel
        virtual void removeChannel(Channel *channel) = 0;
        virtual bool hasChannel(Channel *channel);

    protected:
        using ChannelMap = std::unordered_map<int, Channel*>;
        ChannelMap channels_{};
        EventLoop *ownerLoop;
    };
} // mymuduo


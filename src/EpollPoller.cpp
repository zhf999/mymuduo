//
// Created by zhouhf on 2025/4/15.
//
#include <cerrno>
#include <cstring>

#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

namespace mymuduo{
    // TODO: make these constant more self_explained
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;


    EpollPoller::EpollPoller(EventLoop *loop)
        : Poller(loop),
        epoll_fd_(epoll_create1(EPOLL_CLOEXEC)),
        events_(kInitEventListSize){
        if(epoll_fd_ < 0 )
        {
            Logger::LogFatal("epoll_create error:{}", errno);
        }
    }

    EpollPoller::~EpollPoller() {
        close(epoll_fd_);
    }

    Timestamp EpollPoller::poll(int timeoutMs, mymuduo::Poller::ChannelList *activeChannels) {
        Logger::LogDebug("start epolling...channel count={}",
                         channels_.size());
        int numEvents = epoll_wait(epoll_fd_,events_.data(),static_cast<int>(events_.size()),timeoutMs);
        // 提前存好errno，防止更改其他线程更改
        int saveErrno = errno;
        Timestamp now(Timestamp::now());

        if(numEvents > 0)
        {
            Logger::LogDebug("{} events happened",numEvents);
            fillActiveChannels(numEvents,activeChannels);
            if(numEvents == events_.size())
            {
                events_.resize(events_.size()*2);
            }
        } else if(numEvents == 0)
        {
            Logger::LogDebug("epoll timeout!");
        } else
        {
            if(saveErrno != EINTR)
            {
                errno = saveErrno;
                Logger::LogError("epoll error!");
            }
        }
        return now;
    }

    bool EpollPoller::hasChannel(mymuduo::Channel *channel) {
        return Poller::hasChannel(channel);
    }

    void EpollPoller::removeChannel(mymuduo::Channel *channel) {
        Logger::LogInfo("removing channel from epoll: fd={} events={} index={}",
                        channel->fd(),channel->events(),channel->index());
        int fd = channel->fd();
        int index = channel->index();
        channels_.erase(fd);
        if(index==kAdded)
        {
            update(EPOLL_CTL_DEL, channel);
        }
        channel->set_index(kNew);
    }

    void EpollPoller::updateChannel(mymuduo::Channel *channel) {
        const int index = channel->index();
        Logger::LogInfo("updating channel to epoll: fd={} events={} index={}",
                        channel->fd(),channel->events(),channel->index());
        if(index == kNew || index == kDeleted)
        {
            if(index==kNew)
            {
                int fd = channel->fd();
                channels_[fd] = channel;
            }
            channel->set_index(kAdded);
            update(EPOLL_CTL_ADD,channel);
        }else
        {
            if(channel->isNoneEvent())
            {
                update(EPOLL_CTL_DEL,channel);
                channel->set_index(kDeleted);
            }else
            {
                update(EPOLL_CTL_MOD,channel);
            }
        }
    }

    void EpollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {
        for(int i=0;i<numEvents;i++)
        {
            auto *channel = static_cast<Channel*>(events_[i].data.ptr);
            channel->set_r_events(events_[i].events);
            // channelList里装的是活跃的channel指针
            activeChannels->push_back(channel);
        }
    }

    void EpollPoller::update(int operation, Channel *channel) {
        epoll_event event{};
        event.events = channel->events();
        event.data.ptr = channel;

        if(epoll_ctl(epoll_fd_,operation,channel->fd(),&event)<0)
        {
            if(operation==EPOLL_CTL_DEL)
            {
                Logger::LogError("epoll_ctl failed! op={} fd={} errno={}",
                                 operation, channel->fd(), errno);
            }else
            {
                Logger::LogFatal("epoll_ctl failed! op={} fd={} errno={}",
                                 operation, channel->fd(),errno);
            }
        }
    }


}


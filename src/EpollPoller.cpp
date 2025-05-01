//
// Created by zhouhf on 2025/4/15.
//
#include <cerrno>

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
        LOG_DEBUG("owner_loop=%p, epoll_fd=%d",  loop, epoll_fd_);
        if(epoll_fd_ < 0 )
        {
            LOG_FATAL("epoll_create error=%d", errno);
        }
    }

    EpollPoller::~EpollPoller() {
        LOG_DEBUG("owner_loop=%p, epoll_fd=%d", ownerLoop, epoll_fd_);
        close(epoll_fd_);
    }

    Timestamp EpollPoller::poll(int timeoutMs, mymuduo::Poller::ChannelList *activeChannels) {
        LOG_DEBUG("start polling - channel count=%d",
                         channels_.size());
        int numEvents = epoll_wait(epoll_fd_,events_.data(),static_cast<int>(events_.size()),timeoutMs);
        // 提前存好errno，防止更改其他线程更改
        int saveErrno = errno;
        Timestamp now(Timestamp::now());

        if(numEvents > 0)
        {
            LOG_DEBUG("%d events happened",numEvents);
            fillActiveChannels(numEvents,activeChannels);
            if(numEvents == events_.size())
            {
                events_.resize(events_.size()*2);
            }
        } else if(numEvents == 0)
        {
            LOG_DEBUG("epoll timeout!");
        } else
        {
            if(saveErrno != EINTR)
            {
                errno = saveErrno;
                LOG_ERROR("epoll error!");
            }
        }
        return now;
    }

    bool EpollPoller::hasChannel(mymuduo::Channel *channel) {
        return Poller::hasChannel(channel);
    }

    void EpollPoller::removeChannel(mymuduo::Channel *channel) {
        LOG_INFO("removing channel from epoll: fd=%d events=%d index=%d",
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
        LOG_DEBUG("updating channel to epoll: fd=%d events=%d index=%d",
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

    void EpollPoller::update(int operation, Channel *channel) const {
        epoll_event event{};
        event.events = channel->events();
        event.data.ptr = channel;

        if(epoll_ctl(epoll_fd_,operation,channel->fd(),&event)<0)
        {
            if(operation==EPOLL_CTL_DEL)
            {
                LOG_ERROR("epoll_ctl failed! op=%d fd=%d errno=%d",
                                 operation, channel->fd(), errno);
            }else
            {
                LOG_FATAL("epoll_ctl failed! op=%d fd=%d errno=%d",
                                 operation, channel->fd(),errno);
            }
        }
    }


}


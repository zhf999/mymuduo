//
// Created by zhouhf on 2025/4/12.
//

#pragma once

#include <functional>
#include <memory>
#include "noncopyable.h"
#include "Timestamp.h"

namespace mymuduo {
    class EventLoop;

    class Channel : noncopyable{
    public:
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(Timestamp)>;

        Channel(EventLoop *loop, int fd);
        ~Channel();

        void handleEvent(Timestamp receiveTime);
        void setReadCallback(ReadEventCallback cb){readEventCallback_ = std::move(cb);};
        void setWriteCallback(EventCallback cb){ writeCallback_ = std::move(cb);};
        void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); };
        void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); };

        // 防止当channel被remove掉，channel还在执行回调操作
        void tie(const std::shared_ptr<void>&);

        int fd() const {return fd_;};
        int events() const {return events_;};
        void set_r_events(uint32_t r_evt) { r_events_ = r_evt; };
        bool isNoneEvent() const { return events_ == kNoneEvent; };
        bool isWriting() const { return events_ & kWriteEvent; };
        bool isReading() const { return events_ & kReadEvent; };

        void enableReading() { events_ |= kReadEvent; update(); };
        void disableReading() { events_ &= ~kReadEvent; update(); };
        void enableWriting() { events_ |= kWriteEvent; update(); };
        void disableWriting() { events_ &= ~kWriteEvent; update(); };
        void disableAll() { events_ = kNoneEvent; update(); };

        int index() const { return index_; };
        void set_index(int idx) { index_ = idx; };

        EventLoop* ownerLoop() const { return loop_; };
        void removeFromLoop();

    private:
        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        void update();
        void handleEventWithGuard(Timestamp receiveTime);

        EventLoop *loop_;
        const int fd_;
        int events_;
        uint32_t r_events_;
        int index_;

        std::weak_ptr<void> tie_;
        bool is_tied_;

        ReadEventCallback  readEventCallback_;
        EventCallback writeCallback_, closeCallback_, errorCallback_;
    };

}

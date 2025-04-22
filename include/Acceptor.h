//
// Created by zhouhf on 2025/4/18.
//

#pragma once
#include <functional>
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

namespace mymuduo {
    class EventLoop;
    class InetAddress;

    class Acceptor : noncopyable{
    public:
        using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

        Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort);
        ~Acceptor();

        void setNewConnectionCallback(NewConnectionCallback cb) { newConnectionCallback_ = std::move(cb); }

        bool listening() const { return listening_; };
        void listen();

    private:
        void handleRead(); // acceptSocket有新用户连接
        EventLoop *loop_;
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback  newConnectionCallback_;
        bool listening_{false};
    };

} // mymuduo


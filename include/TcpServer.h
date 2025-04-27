//
// Created by zhouhf on 2025/4/12.
//
#pragma once
#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "Buffer.h"

namespace mymuduo {

    class TcpServer : noncopyable{
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;
        enum Option{
            kNoReusePort,
            kReusePort,
        };

        TcpServer(EventLoop *sock,
                  const InetAddress &peerAddr,
                  std::string name,
                  Option option = kNoReusePort);
        ~TcpServer();

        void setThreadInitCallback(ThreadInitCallback cb) { threadInitCallback_ = std::move(cb); };
        void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); };
        void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb);};
        void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); };

        void setThreadNum(int numThreads);
        const std::string& name() { return name_; };

        void start();
    private:
        void handleNewConnection(int sockfd, const InetAddress &peerAddr);
        void removeConnection(const TcpConnectionPtr &conn);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

        using ConnectionMap = std::unordered_map<std::string ,TcpConnectionPtr >;
        EventLoop *loop_;
        const std::string ipPort_;
        const std::string name_;
        std::unique_ptr<Acceptor> acceptor_;
        std::shared_ptr<EventLoopThreadPool> threadPool_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        ThreadInitCallback threadInitCallback_;

        std::atomic_bool started_{false};
        int nextConnId_{1};
        ConnectionMap connections_{};
    };

} // mymuduo

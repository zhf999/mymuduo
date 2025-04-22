//
// Created by zhouhf on 2025/4/19.
//
#pragma once
#include <string>
#include <atomic>
#include <memory>
#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

namespace mymuduo {
    class Channel;
    class EventLoop;
    class Socket;

    class TcpConnection :noncopyable, public std::enable_shared_from_this<TcpConnection>{
    public:
        TcpConnection(EventLoop *loop,
                      std::string name,
                      int sockfd,
                      const InetAddress &localAddr,
                      const InetAddress &peerAddr);
        ~TcpConnection();

        EventLoop* getLoop() const { return loop_; }
        const std::string& name() const { return name_; }
        const InetAddress& localAddress() const { return localAddr_; }
        const InetAddress& peerAddress() const { return peerAddr_; }

        bool connected() { return state_ = kConnected; }

        void send(const std::string& buf);
        void shutdown();

        // 连接建立和断开都会调用
        void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); };
        void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb);};
        void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); };
        void setHighWaterMarkCallback(HighWaterMarkCallback cb) { highWaterMarkCallback_ = std::move(cb); };
        void setCloseCallback(CloseCallback cb) { closeCallback_ = std::move(cb); };

        void connectionEstablished();
        void connectDestroyed();

    private:
        enum EnumState{ kDisconnected, kConnecting, kConnected, kDisconnecting};

        void setState(EnumState state) { state_ = state; };
        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();

        void sendInLoop(const void* message, size_t len);
        void shutdownInLoop();

        EventLoop *loop_;
        const std::string name_;
        std::atomic_int state_{kConnecting};

        bool reading_{true};
        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;

        const InetAddress localAddr_;
        const InetAddress peerAddr_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        HighWaterMarkCallback highWaterMarkCallback_;
        CloseCallback closeCallback_;
        size_t highWaterMark_{64*1024*1024}; // 64MBytes

        Buffer inputBuffer_;
        Buffer outputBuffer_;
    };

} // mymuduo


//
// Created by zhouhf on 2025/4/18.
//

#pragma once

#include "noncopyable.h"

namespace mymuduo {
//    class InetAddress;
    class InetAddress;
    class Socket : noncopyable{
    public:
        explicit Socket(int sockfd):sockfd_(sockfd) {};
        ~Socket();

        int fd() const { return sockfd_; };
        void bindAddress(const InetAddress &localAddr) const;
        void listen() const;
        int accept(InetAddress *peerAddr) const;

        void shutdownWrite() const;

        void setTcpNoDelay(bool on) const;
        void setReuseAddr(bool on) const;
        void setReusePort(bool on) const;
        void setKeepAlive(bool on) const;
    private:
        const int sockfd_{};
    };

} // mymuduo

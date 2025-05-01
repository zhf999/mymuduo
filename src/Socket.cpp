//
// Created by zhouhf on 2025/4/18.
//

#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <netinet/tcp.h>

#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"



namespace mymuduo {
    mymuduo::Socket::~Socket() {
        close(sockfd_);
    }

    void Socket::bindAddress(const InetAddress &localAddr) const{
        if(0 != bind(sockfd_,(sockaddr*)localAddr.getSockAddr(),sizeof(sockaddr_in)))
        {
            LOG_FATAL("bind sockfd fd=%d fail",sockfd_);
        }
    }

    void Socket::listen() const {
        if(0 != ::listen(sockfd_,1024))
        {
            LOG_FATAL("listen sockfd fd=%d fail",sockfd_);
        }
    }

    int Socket::accept(InetAddress *peerAddr) const {
        sockaddr_in addr{};
        socklen_t socklen = sizeof(addr);
        int connFd = ::accept4(sockfd_, (sockaddr*)&addr, &socklen, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(connFd >= 0)
        {
            peerAddr->setSockAddr(addr);
        }
        return connFd;
    }

    void Socket::shutdownWrite() const {
        if(::shutdown(sockfd_,SHUT_WR) < 0)
        {
            LOG_ERROR("shutdown write fd=%d error",sockfd_);
        }
    }

    void Socket::setTcpNoDelay(bool on) const {
        int opt_val = on;
        ::setsockopt(sockfd_,IPPROTO_TCP, TCP_NODELAY,&opt_val,sizeof(opt_val));
    }

    void Socket::setReuseAddr(bool on) const {
        int opt_val = on;
        ::setsockopt(sockfd_,SOL_SOCKET, SO_REUSEADDR,&opt_val,sizeof(opt_val));
    }

    void Socket::setReusePort(bool on) const {
        int opt_val = on;
        ::setsockopt(sockfd_,SOL_SOCKET, SO_REUSEPORT,&opt_val,sizeof(opt_val));
    }

    void Socket::setKeepAlive(bool on) const {
        int opt_val = on;
        ::setsockopt(sockfd_,SOL_SOCKET, SO_KEEPALIVE,&opt_val,sizeof(opt_val));
    }
} // mymuduo
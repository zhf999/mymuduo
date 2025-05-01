//
// Created by zhouhf on 2025/4/18.
//

#include <sys/socket.h>
#include <cerrno>
#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

namespace mymuduo {
    static int createNonBlocking()
    {
        int sockfd = ::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC, 0);
        if(sockfd<0)
        {
            LOG_FATAL("create socket fail at createNonBlocking, errno=%d", errno);
        }
        return sockfd;
    }

    Acceptor::~Acceptor() {
        LOG_DEBUG("ownerLoop=%p", loop_);
        acceptChannel_.disableAll();
        acceptChannel_.removeFromLoop();
    }

    Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort)
    :loop_(loop),
     acceptSocket_(createNonBlocking()),
     acceptChannel_(loop, acceptSocket_.fd())
    {
        LOG_DEBUG("ownerLoop=%p addr=%s", static_cast<void*>(loop), listenAddr.toIpPort().c_str());
        acceptSocket_.setReuseAddr(true);
        acceptSocket_.setReusePort(reusePort);
        acceptSocket_.bindAddress(listenAddr);
        acceptChannel_.setReadCallback([&](Timestamp timestamp){this->handleRead();});
    }

    void Acceptor::handleRead() {
        LOG_DEBUG("owner loop %p",loop_);
        InetAddress peerAddr;
        int conn_fd = acceptSocket_.accept(&peerAddr);
        if(conn_fd >= 0)
        {
            if(newConnectionCallback_)
                newConnectionCallback_(conn_fd,peerAddr);
            else
            {
                ::close(conn_fd);
            }
        }else
        {
            LOG_ERROR("accept fail, errno=%d",errno);
        }
    }

    void Acceptor::listen() {
        LOG_INFO("Acceptor::listen - ownerLoop:{}", static_cast<void*>(loop_));
        listening_ = true;
        acceptSocket_.listen();
        acceptChannel_.enableReading();
    }


} // mymuduo
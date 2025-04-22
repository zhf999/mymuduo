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
            Logger::LogFatal("create socket fail at createNonBlocking, errno:{}", errno);
        }
        return sockfd;
    }

    Acceptor::~Acceptor() {
        acceptChannel_.disableAll();
        acceptChannel_.removeFromLoop();
    }

    Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort)
    :loop_(loop),
     acceptSocket_(createNonBlocking()),
     acceptChannel_(loop, acceptSocket_.fd())
    {
        Logger::LogDebug("Acceptor::init - ownerLoop:{} addr:{}", static_cast<void*>(loop), listenAddr.toIpPort());
        acceptSocket_.setReuseAddr(true);
        acceptSocket_.setReusePort(reusePort);
        acceptSocket_.bindAddress(listenAddr);
        acceptChannel_.setReadCallback([&](Timestamp timestamp){this->handleRead();});
    }

    void Acceptor::handleRead() {
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
            Logger::LogError("{}: accept fail, errno:{}",__FUNCTION__,errno);
        }
    }

    void Acceptor::listen() {
        listening_ = true;
        acceptSocket_.listen();
        acceptChannel_.enableReading();
    }


} // mymuduo
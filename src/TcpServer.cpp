//
// Created by zhouhf on 2025/4/12.
//

#include <format>
#include "TcpServer.h"
#include "Logger.h"
// TODO: rethink use of lambda and std::bind
namespace mymuduo {
    static EventLoop* CheckLoopNotNull(EventLoop *loop)
    {
        if(loop == nullptr)
        {
            LOG_FATAL("Loop pointer can't be nullptr!");
        }
        return loop;
    }
    TcpServer::TcpServer(EventLoop *loop,
                         const InetAddress &listenAddr,
                         std::string name,
                         TcpServer::Option option)
    :loop_(CheckLoopNotNull(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(std::move(name)),
    acceptor_(std::make_unique<Acceptor>(loop,listenAddr,option==kReusePort)),
    threadPool_(std::make_shared<EventLoopThreadPool>(loop,name_))
    {
        LOG_INFO("name=%s", name_.c_str(),ipPort_.c_str());
        acceptor_->setNewConnectionCallback(
                [this](auto sock, auto peerAddr)
                {
                    this->handleNewConnection(sock, peerAddr);
                }
                );
    }

    TcpServer::~TcpServer() {
        LOG_DEBUG("name=%s",name_.c_str(), ipPort_.c_str());
        for(auto &item:connections_)
        {
            TcpConnectionPtr conn(item.second);
            item.second.reset();
            conn->getLoop()->runInLoop(
                    [conn](){
                        conn->connectDestroyed();
                    }
                    );
        }
    }

    void TcpServer::handleNewConnection(int sockfd, const InetAddress &peerAddr) {
        LOG_INFO("new connection from addr=%s, fd=%d",peerAddr.toIpPort().c_str(),sockfd);

        EventLoop *ioLoop = threadPool_->getNextLoop();
        std::string connName = std::format("{}-{}#{}",name_,peerAddr.toIpPort(),nextConnId_);
        nextConnId_++;

        sockaddr_in local{};
        socklen_t addrLen = sizeof(local);
        if(::getsockname(sockfd,(sockaddr*)&local, &addrLen)<0)
        {
            LOG_ERROR("sockets: getLocalAddr fail!");
        }
        InetAddress localAddr(local);
        TcpConnectionPtr connectionPtr = std::make_shared<TcpConnection>(
                ioLoop,
                connName,
                sockfd,
                localAddr,
                peerAddr
                );

        connections_[connName] = connectionPtr;
        connectionPtr->setConnectionCallback(connectionCallback_);
        connectionPtr->setMessageCallback(messageCallback_);
        connectionPtr->setWriteCompleteCallback(writeCompleteCallback_);
        connectionPtr->setCloseCallback(
                [this](const TcpConnectionPtr &connection){
                    this->removeConnection(connection);
                }
                );

        ioLoop->runInLoop(
                [connectionPtr](){
                    connectionPtr->connectionEstablished();
                }
                );
    }

    void TcpServer::setThreadNum(int numThreads) {
        threadPool_->setThreadNum(numThreads);
    }

    void TcpServer::start() {
        if(!started_)
        {
            LOG_INFO("server name=%s started",name_.c_str());
            started_ = true;
            threadPool_->start(threadInitCallback_);
            loop_->runInLoop(
                    [this](){
                     acceptor_->listen();
                    });
        }
    }

    void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
        LOG_DEBUG("remove connection from %s",conn->peerAddress().toIpPort().c_str());
        loop_->runInLoop(
                [conn,this](){
                    removeConnectionInLoop(conn);
                }
                );
//        loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
    }

    void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
        LOG_INFO("remove connection from %s",conn->peerAddress().toIpPort().c_str());
        connections_.erase(conn->name());
        EventLoop *ioLoop = conn->getLoop();
        ioLoop->queueInLoop(
                [conn](){
                    conn->connectDestroyed();
                }
                );
    }

} // mymuduo
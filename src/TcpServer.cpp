//
// Created by zhouhf on 2025/4/12.
//

#include "TcpServer.h"
#include "Logger.h"
// TODO: rethink use of lambda and std::bind
namespace mymuduo {
    static EventLoop* CheckLoopNotNull(EventLoop *loop)
    {
        if(loop == nullptr)
        {
            Logger::LogFatal("Loop pointer can't be nullptr!");
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
        Logger::LogDebug("TcpServer::constructor - name:{} addr:{}", name_,ipPort_);
        acceptor_->setNewConnectionCallback(
                [this](auto sock, auto peerAddr)
                {
                    this->handleNewConnection(sock, peerAddr);
                }
                );
    }

    TcpServer::~TcpServer() {
        Logger::LogDebug("TcpServer::destructor - name:{} addr:{}",name_, ipPort_);
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
        EventLoop *ioLoop = threadPool_->getNextLoop();
        std::string connName = std::format("{}-{}#{}",name_,ipPort_,nextConnId_);
        nextConnId_++;
        Logger::LogInfo("TcpServer [{}]::new connection [{}] from {}",
                        name_, connName, peerAddr.toIpPort());

        sockaddr_in local{};
        socklen_t addrLen = sizeof(local);
        if(::getsockname(sockfd,(sockaddr*)&local, &addrLen)<0)
        {
            Logger::LogError("sockets: getLocalAddr fail!");
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
            Logger::LogInfo("TcpServer::start - server name:{}, addr:{}",name_,ipPort_);
            started_ = true;
            threadPool_->start(threadInitCallback_);
            loop_->runInLoop(
                    [this](){
                     acceptor_->listen();
                    });
        }
    }

    void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
        Logger::LogDebug("TcpServer::removeConnection - remove connection from {}",conn->peerAddress().toIpPort());
        loop_->runInLoop(
                [conn,this](){
                    removeConnectionInLoop(conn);
                }
                );
//        loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
    }

    void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
        Logger::LogInfo("TcpServer::removeConnectionInLoop - remove connection from {}",conn->peerAddress().toIpPort());
        connections_.erase(conn->name());
        EventLoop *ioLoop = conn->getLoop();
        ioLoop->queueInLoop(
                [conn](){
                    conn->connectDestroyed();
                }
                );
    }

} // mymuduo
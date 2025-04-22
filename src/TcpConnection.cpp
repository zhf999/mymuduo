//
// Created by zhouhf on 2025/4/19.
//

#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include <cerrno>

namespace mymuduo {
    TcpConnection::TcpConnection(EventLoop *loop, std::string name, int sockfd, const InetAddress &localAddr,
                                 const InetAddress &peerAddr)
                                 :loop_(loop),
                                 name_(std::move(name)),
                                 socket_(std::make_unique<Socket>(sockfd)),
                                 channel_(std::make_unique<Channel>(loop,sockfd)),
                                 localAddr_(localAddr),
                                 peerAddr_(peerAddr){
        channel_->setReadCallback(
                [this](Timestamp timestamp){
                    this->handleRead(timestamp);
                }
                );

        channel_->setWriteCallback(
                [this]()
                {
                    this->handleWrite();
                }
                );
        channel_->setCloseCallback(
                [this](){
                    this->handleClose();
                }
                );
        channel_->setErrorCallback(
                [this]()
                {
                    this->handleError();
                }
                );

        Logger::LogInfo("TcpConnection created {} at fd={}",name_,sockfd);
        socket_->setKeepAlive(true);

    }

    TcpConnection::~TcpConnection() {
        Logger::LogInfo("TcpConnection dis-created {} at fd={}",name_,channel_->fd());
    }

    void TcpConnection::handleRead(Timestamp receiveTime) {
        int savedErrno = 0;
        ssize_t n = inputBuffer_.readFd(socket_->fd(), &savedErrno);
        if(n>0)
        {
            Logger::LogDebug("TcpConnection::handleRead peer{} - buffer:{}", peerAddr_.toIpPort() ,std::string(inputBuffer_.peek(),inputBuffer_.readableBytes()));
            messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
        }else if(n==0)
        {
            Logger::LogDebug("TcpConnection::handleRead peer:{} - receive FIN",peerAddr_.toIpPort());
            handleClose();
        }else
        {
            errno = savedErrno;
            Logger::LogError("TcpConnection::handleRead");
            handleError();
        }
    }

    void TcpConnection::handleWrite() {
        if(channel_->isWriting())
        {
            int saveErrno = 0;
            ssize_t n = outputBuffer_.writeFd(socket_->fd(),&saveErrno);
            if(n>0)
            {
                if(outputBuffer_.readableBytes()==0)
                {
                    channel_->disableWriting();
                    if(writeCompleteCallback_)
                    {
                        loop_->queueInLoop(
                                std::bind(writeCompleteCallback_,shared_from_this())
                        );
                    }
                    if(state_==kDisconnecting)
                    {
                        shutdownInLoop();
                    }
                }
            }
            else
            {
                Logger::LogError("TcpConnection::handwrite fail! errno:{} ",saveErrno);
            }
        } else
        {
            Logger::LogError("Tcp connection fd={} is down, no more writing", socket_->fd());
        }
    }

    void TcpConnection::handleClose() {
        Logger::LogInfo("TcpConnection::handleClose peer:{}",peerAddr_.toIpPort());
        setState(kDisconnected);
        channel_->disableAll();
        TcpConnectionPtr connPtr(shared_from_this());
        connectionCallback_(connPtr); // TODO: why conn callback?
        closeCallback_(connPtr);
    }

    void TcpConnection::handleError() {
        int optVal;
        socklen_t optLen = sizeof(optVal);
        int err = 0;
        if(::getsockopt(socket_->fd(),SOL_SOCKET,SO_ERROR,&optVal,&optLen)<0){
            err = errno;
        }else
        {
            err = optVal;
        }
        Logger::LogError("TcpConnection::handleError name:{} - SO_ERROR:{}",
                         name_,err);
    }

    void TcpConnection::send(const std::string &buf) {
        if(state_==kConnected)
        {
            if(loop_->isInLoopThread())
            {
                sendInLoop(buf.c_str(),buf.size());
            }else
            {
                loop_->runInLoop(
                        [this,buf](){
                            sendInLoop(buf.c_str(),buf.size());
                        }
                        );
            }
        }
    }

    void TcpConnection::sendInLoop(const void *message, size_t len) {
        ssize_t n_wrote = 0;
        size_t remaining = len;
        bool faultError = false;

        if(state_==kDisconnected)
        {
            Logger::LogError("TcpConnection disconnected, give up writing");
        }

        // channel第一次开始写数据，并且缓冲区没有待发送数据
        if(!channel_->isWriting()&&outputBuffer_.readableBytes()==0)
        {
            n_wrote = ::write(socket_->fd(),message,len);
            if(n_wrote>=0)
            {
                remaining = len - n_wrote;
                if(remaining==0&&writeCompleteCallback_)
                {
                    loop_->queueInLoop(
                            [this](){
                                writeCompleteCallback_(shared_from_this());
                            }
                            );
                }
            }else
            {
                n_wrote = 0;
                if(errno!=EWOULDBLOCK)
                {
                    Logger::LogError("TcpConnection::sendInLoop");
                    if(errno==EPIPE||errno==ECONNRESET)
                    {
                        faultError = true;
                    }
                }
            }
        }

        // 没有一次性发送完
        if(!faultError&&remaining>0)
        {
            size_t oldLen = outputBuffer_.readableBytes();
            if(oldLen+remaining>=highWaterMark_
            &&oldLen<highWaterMark_
            &&highWaterMarkCallback_)
            {
                loop_->queueInLoop(
                        [this,oldLen,remaining](){
                            highWaterMarkCallback_(shared_from_this(),oldLen+remaining);
                        }
                        );
            }
            outputBuffer_.append((char*)message+n_wrote,remaining);
            if(channel_->isWriting())
                channel_->enableWriting();
        }
    }

    void TcpConnection::connectionEstablished() {
        setState(kConnected);
        channel_->tie(shared_from_this());
        channel_->enableReading();

        connectionCallback_(shared_from_this());
    }

    void TcpConnection::connectDestroyed() {
        Logger::LogDebug("TcpConnection::connectDestroyed - peer:{} state:{}",peerAddr_.toIpPort(),state_.load());
        if(state_==kConnected)
        {
            setState(kDisconnected);
            channel_->disableAll();
            connectionCallback_(shared_from_this());
        }
        channel_->removeFromLoop();
    }

    void TcpConnection::shutdown() {
        if(state_==kConnected)
        {
            setState(kDisconnecting);
            loop_->runInLoop([this](){
                shutdownInLoop();
            });
        }
    }

    void TcpConnection::shutdownInLoop() {
        if(!channel_->isWriting())
        {
            socket_->shutdownWrite();
        }
    }


} // mymuduo
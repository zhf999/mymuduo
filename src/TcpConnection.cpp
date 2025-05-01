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

        LOG_INFO("name=%s, fd=%d",name_.c_str(),sockfd);
        socket_->setKeepAlive(true);

    }

    TcpConnection::~TcpConnection() {
        LOG_INFO("name=%s, fd=%d",name_.c_str(),channel_->fd());
    }

    void TcpConnection::handleRead(Timestamp receiveTime) {
        LOG_INFO("name=%s, fd=%d",name_.c_str(),channel_->fd());
        int savedErrno = 0;
        ssize_t n = inputBuffer_.readFd(socket_->fd(), &savedErrno);
        if(n>0)
        {
            LOG_DEBUG("buffer=%s",std::string(inputBuffer_.peek(),inputBuffer_.readableBytes()).c_str());
            messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
        }else if(n==0)
        {
            LOG_DEBUG("receive FIN");
            handleClose();
        }else
        {
            errno = savedErrno;
            LOG_ERROR("errno=%d",errno);
            handleError();
        }
    }

    void TcpConnection::handleWrite() {
        LOG_INFO("name=%s, fd=%d",name_.c_str(),channel_->fd());
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
                LOG_ERROR("conn name=%s errno=%d ",name_.c_str(), saveErrno);
            }
        } else
        {
            LOG_ERROR("conn %s is down, no more writing", name_.c_str());
        }
    }

    void TcpConnection::handleClose() {
        LOG_INFO("conn name=%s", name_.c_str());
        setState(kDisconnected);
        channel_->disableAll(); // TODO: What if there is data in outputBuffer_?
        TcpConnectionPtr connPtr(shared_from_this());
        connectionCallback_(connPtr);
        closeCallback_(connPtr);
    }

    void TcpConnection::handleError() {
        int optVal;
        socklen_t optLen = sizeof(optVal);
        int err;
        if(::getsockopt(socket_->fd(),SOL_SOCKET,SO_ERROR,&optVal,&optLen)<0){
            err = errno;
        }else
        {
            err = optVal;
        }
        LOG_ERROR("conn name=%s errno=%d",
                         name_.c_str(),err);
    }

    void TcpConnection::send(const std::string &buf) {
        LOG_DEBUG("schedule `send` conn name=%s, msg=%s",name_.c_str(),buf.substr(0,10).c_str());
        if(state_==kConnected)
        {
            // TODO: duplicated if statement
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
        LOG_INFO("start `send` conn name=%s", name_.c_str());

        ssize_t n_wrote = 0;
        size_t remaining = len;
        bool faultError = false;

        if(state_==kDisconnected)
        {
            LOG_ERROR("conn name=%s is down, give up writing",name_.c_str());
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
                    LOG_ERROR("conn name=%s",name_.c_str());
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
            if(!channel_->isWriting())
                channel_->enableWriting();
        }
    }

    void TcpConnection::connectionEstablished() {
        LOG_DEBUG("conn name=%s",name_.c_str());
        setState(kConnected);
        channel_->tie(shared_from_this());
        channel_->enableReading();

        connectionCallback_(shared_from_this());
    }

    // TODO: this can be invoked in Destructor?
    void TcpConnection::connectDestroyed() {
        LOG_DEBUG("conn name:%s state:%d",peerAddr_.toIpPort().c_str(),state_.load());
        if(state_==kConnected)
        {
            setState(kDisconnected);
            channel_->disableAll();
            connectionCallback_(shared_from_this());
        }
        channel_->removeFromLoop();
    }

    void TcpConnection::shutdown() {
        LOG_DEBUG("conn name %s will closed by server",name_.c_str());
        if(state_==kConnected)
        {
            setState(kDisconnecting);
            loop_->runInLoop([this](){
                shutdownInLoop();
            });
        }
    }

    void TcpConnection::shutdownInLoop() {
        LOG_DEBUG("conn name %s is closed by server",name_.c_str());
        if(!channel_->isWriting())
        {
            socket_->shutdownWrite();
        }
    }


} // mymuduo
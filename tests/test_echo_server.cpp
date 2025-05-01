//
// Created by zhouhf on 2025/4/21.
//
#include "TcpServer.h"
#include "Logger.h"
#include <iostream>

using namespace mymuduo;
using namespace std::placeholders;

class EchoServer{
public:
    EchoServer(EventLoop *loop,
               const InetAddress& inetAddress,
               const std::string& nameArg) : _server(loop, inetAddress,nameArg),
                                             _loop(loop)
    {
        _server.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
        _server.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
        _server.setThreadNum(1);
    }

    void start()
    {
        _server.start();
    }
private:
    void onConnection(const TcpConnectionPtr& conn) {
        if(conn->connected()) {
//            Logger::LogInfo("{}: Connection {} is UP",_server.name(), conn->name());
        } else{
//            Logger::LogInfo("{}: Connection {} is DOWN",_server.name(), conn->name());
            conn->shutdown();
        }
    }

    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buffer,
                   Timestamp timestamp) {
        std::string buf = buffer->retrieveAllAsString();
        LOG_INFO("Receive string %s from connection %s", buf.c_str(), conn->name().c_str());
        conn->send(buf);
    }

    TcpServer _server;
    EventLoop *_loop;
};

int main() {
    Logger::instance().setStandardLevel(Logger::LogLevel::INFO);
    EventLoop loop;
    InetAddress addr("127.0.0.1",8080);
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    loop.loop();
    return 0;
}


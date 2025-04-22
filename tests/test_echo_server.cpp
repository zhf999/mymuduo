//
// Created by zhouhf on 2025/4/21.
//
#include "TcpServer.h"
#include "Logger.h"
#include <iostream>

using namespace mymuduo;
using namespace std::placeholders;

class ChatServer{
public:
    ChatServer(EventLoop *loop,
               const InetAddress& inetAddress,
               const std::string& nameArg) : _server(loop, inetAddress,nameArg),
                                             _loop(loop)
    {
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
        _server.setThreadNum(0);
    }

    void start()
    {
        _server.start();
    }
private:
    void onConnection(const TcpConnectionPtr& conn) {
        std::cout<<conn->peerAddress().toIpPort()<< " -> "
                 << conn->localAddress().toIpPort();
        if(conn->connected()) {
            std::cout << " [state: online] " << std::endl;
        } else{
            std::cout << " [state: offline] " << std::endl;
            conn->shutdown();
        }
    }

    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buffer,
                   Timestamp timestamp) {
        std::string buf = buffer->retrieveAllAsString();
        std::cout << "receive data: " << buf << " time: " << timestamp.toString() << std::endl;
        conn->send(buf);
    }

    TcpServer _server;
    EventLoop *_loop;
};

int main() {
    Logger::instance().setStandardLevel(Logger::LogLevel::DEBUG);
    EventLoop loop;
    InetAddress addr("127.0.0.1",8080);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
    return 0;
}


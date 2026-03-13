//
// Created by zhouhf on 2025/5/2.
//

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "HttpContext.h"
#include "HttpResponse.h"
#include "TcpServer.h"

namespace mymuduo::http {
    class HttpServer {
    public:
        using HttpCallback = std::function<void(const HttpRequest &, HttpResponse *)>;

        HttpServer(EventLoop *loop,
                   const InetAddress &listenAddr,
                   std::string name,
                   TcpServer::Option option = TcpServer::kNoReusePort);

        void setThreadNum(int numThreads);
        void start();

        void setHttpCallback(HttpCallback cb) { requestCallback_ = std::move(cb); }

    private:
        void onConnection(const TcpConnectionPtr &conn);
        void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime);
        void onRequest(const TcpConnectionPtr &conn, const HttpRequest &request);

        static void defaultHttpCallback(const HttpRequest &request, HttpResponse *response);

        TcpServer tcpServer_;
        HttpCallback requestCallback_;
    };
} // namespace mymuduo::http

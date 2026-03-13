//
// Created by zhouhf on 2025/5/2.
//

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

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

        std::shared_ptr<HttpContext> getOrCreateContext(const std::string &connectionName);
        void removeContext(const std::string &connectionName);

        TcpServer tcpServer_;
        HttpCallback requestCallback_;

        std::mutex contextsMutex_;
        std::unordered_map<std::string, std::shared_ptr<HttpContext>> contexts_;
    };
} // namespace mymuduo::http

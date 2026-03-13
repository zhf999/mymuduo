//
// Created by zhouhf on 2025/5/2.
//

#include "http/HttpServer.h"

#include <algorithm>
#include <cctype>

namespace {
    std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    bool shouldCloseConnection(const mymuduo::http::HttpRequest &request) {
        const std::string connection = toLower(request.getHeaderValue("connection"));
        if (request.getVersion() == mymuduo::http::EnumHttpVersion::HTTP_10) {
            return connection != "keep-alive";
        }
        return connection == "close";
    }
} // namespace

namespace mymuduo::http {
    HttpServer::HttpServer(EventLoop *loop,
                           const InetAddress &listenAddr,
                           std::string name,
                           TcpServer::Option option)
        : tcpServer_(loop, listenAddr, std::move(name), option),
          requestCallback_(defaultHttpCallback),
          contextsMutex_(),
          contexts_() {
        using namespace std::placeholders;
        tcpServer_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
        tcpServer_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
    }

    void HttpServer::setThreadNum(int numThreads) {
        tcpServer_.setThreadNum(numThreads);
    }

    void HttpServer::start() {
        tcpServer_.start();
    }

    void HttpServer::onConnection(const TcpConnectionPtr &conn) {
        if (!conn->connected()) {
            removeContext(conn->name());
        }
    }

    void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        std::shared_ptr<HttpContext> context = getOrCreateContext(conn->name());
        const bool parseResult = context->parse(buf, receiveTime);

        if (!parseResult) {
            HttpResponse response(true);
            response.setStatusCode(HttpResponse::EnumStatusCode::k400BadRequest);
            response.setStatusMessage("Bad Request");
            response.setContentType("text/plain; charset=utf-8");
            response.setBody("400 Bad Request\n");
            conn->send(response.toString());
            conn->shutdown();
            removeContext(conn->name());
            return;
        }

        if (!context->isFinished()) {
            return;
        }

        onRequest(conn, context->request());
        context->reset();
    }

    void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &request) {
        HttpResponse response(shouldCloseConnection(request));
        response.setVersion(request.getVersion());

        requestCallback_(request, &response);

        conn->send(response.toString());
        if (response.closeConnection()) {
            conn->shutdown();
        }
    }

    void HttpServer::defaultHttpCallback(const HttpRequest &request, HttpResponse *response) {
        response->setStatusCode(HttpResponse::EnumStatusCode::k404NotFound);
        response->setStatusMessage("Not Found");
        response->setContentType("text/plain; charset=utf-8");
        response->setBody("404 Not Found: " + request.getUrl() + "\n");
    }

    std::shared_ptr<HttpContext> HttpServer::getOrCreateContext(const std::string &connectionName) {
        std::lock_guard<std::mutex> lock(contextsMutex_);
        auto it = contexts_.find(connectionName);
        if (it == contexts_.end()) {
            auto [newIt, inserted] = contexts_.emplace(connectionName, std::make_shared<HttpContext>());
            (void) inserted;
            return newIt->second;
        }
        return it->second;
    }

    void HttpServer::removeContext(const std::string &connectionName) {
        std::lock_guard<std::mutex> lock(contextsMutex_);
        contexts_.erase(connectionName);
    }
} // namespace mymuduo::http

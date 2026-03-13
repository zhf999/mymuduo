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
          requestCallback_(defaultHttpCallback) {
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
        if (conn->connected()) {
            conn->setContext(std::make_shared<HttpContext>());
        } else {
            conn->setContext(nullptr);
        }
    }

    void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        std::shared_ptr<HttpContext> context = std::static_pointer_cast<HttpContext>(conn->getContext());
        if (!context) {
            context = std::make_shared<HttpContext>();
            conn->setContext(context);
        }
        const bool parseResult = context->parse(buf, receiveTime);

        if (!parseResult) {
            HttpResponse response(true);
            response.setStatusCode(HttpResponse::EnumStatusCode::k400BadRequest);
            response.setStatusMessage("Bad Request");
            response.setContentType("text/plain; charset=utf-8");
            response.setBody("400 Bad Request\n");
            conn->send(response.toString());
            conn->shutdown();
            conn->setContext(nullptr);
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
} // namespace mymuduo::http

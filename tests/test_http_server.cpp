#include <cstdint>
#include <cstdlib>
#include <memory>

#include "EventLoop.h"
#include "InetAddress.h"
#include "Logger.h"
#include "http/HttpResponse.h"
#include "http/HttpServer.h"

using namespace mymuduo;
using namespace mymuduo::http;

namespace {
    void onHttpRequest(const HttpRequest &request, HttpResponse *response) {
        if (request.getUrl() == "/") {
            response->setStatusCode(HttpResponse::EnumStatusCode::k200Ok);
            response->setStatusMessage("OK");
            response->setContentType("text/plain; charset=utf-8");
            response->setBody("mymuduo http server is running\n");
            return;
        }

        if (request.getUrl() == "/echo" && request.getMethod() == EnumHttpMethod::POST) {
            response->setStatusCode(HttpResponse::EnumStatusCode::k200Ok);
            response->setStatusMessage("OK");
            response->setContentType("text/plain; charset=utf-8");
            response->setBody(request.getBody());
            return;
        }

        response->setStatusCode(HttpResponse::EnumStatusCode::k404NotFound);
        response->setStatusMessage("Not Found");
        response->setContentType("text/plain; charset=utf-8");
        response->setBody("404 Not Found\n");
    }
} // namespace

int main(int argc, char *argv[]) {
    int port = 8080;
    int threadNum = 1;

    if (argc > 1) {
        port = std::atoi(argv[1]);
        if (port <= 0) {
            port = 8080;
        }
    }

    if (argc > 2) {
        threadNum = std::atoi(argv[2]);
        if (threadNum <= 0) {
            threadNum = 0;
        }
    }

    Logger::instance().setStandardLevel(Logger::LogLevel::ERROR);

    EventLoop loop;
    InetAddress listenAddress("0.0.0.0", static_cast<uint16_t>(port));
    HttpServer server(&loop, listenAddress, "HttpServer");
    server.setThreadNum(threadNum);
    server.setHttpCallback(onHttpRequest);

    LOG_INFO("HttpServer listening on port=%d threads=%d", port, threadNum);

    server.start();
    loop.loop();
    return 0;
}

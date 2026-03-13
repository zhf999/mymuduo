//
// Created by zhouhf on 2025/5/2.
//

#pragma once

#include "HttpContext.h"
#include "TcpServer.h"

namespace mymuduo {
    namespace http {
        class HttpServer {
        public:
            using RequestCallback = std::function<void(HttpRequest*)>;
        private:
            TcpServer tcpServer_;
            HttpContext context_;
            RequestCallback requestCallback_;
        };
    }
} // mymuduo


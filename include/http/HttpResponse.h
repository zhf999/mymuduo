//
// Created by zhouhf on 2025/5/2.
//

#pragma once
#include <string>
#include <unordered_map>

#include "HttpRequest.h"

namespace mymuduo{
    class Buffer;
    namespace http{
        class HttpResponse {
        public:
            enum class EnumStatusCode {
                kUnknown,
                k200Ok = 200,
                k400BadRequest = 400,
                k404NotFound = 404,
                k500InternalServerError = 500,
            };

            void writeToBuffer(Buffer *buf);
            std::string toString() const;
        private:
            EnumHttpVersion version_;
            EnumStatusCode statusCode_;
            std::string statusMessage_;
            std::unordered_map<std::string, std::string> headers_;
            std::string responseBody_;
        };
    }
}



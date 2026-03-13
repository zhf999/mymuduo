//
// Created by zhouhf on 2025/5/2.
//

#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include "HttpRequest.h"

namespace mymuduo {
    class Buffer;

    namespace http {
        class HttpResponse {
        public:
            enum class EnumStatusCode {
                kUnknown,
                k200Ok = 200,
                k400BadRequest = 400,
                k404NotFound = 404,
                k500InternalServerError = 500,
            };

            explicit HttpResponse(bool close = true);

            void setVersion(EnumHttpVersion version) { version_ = version; }
            EnumHttpVersion getVersion() const { return version_; }

            void setStatusCode(EnumStatusCode statusCode) { statusCode_ = statusCode; }
            EnumStatusCode getStatusCode() const { return statusCode_; }

            void setStatusMessage(std::string statusMessage) { statusMessage_ = std::move(statusMessage); }
            const std::string &getStatusMessage() const { return statusMessage_; }

            void setCloseConnection(bool close) { closeConnection_ = close; }
            bool closeConnection() const { return closeConnection_; }

            void setContentType(const std::string &contentType);
            void addHeader(std::string key, std::string value);

            void setBody(std::string body) { responseBody_ = std::move(body); }
            const std::string &getBody() const { return responseBody_; }

            void writeToBuffer(Buffer *buf);
            std::string toString() const;

        private:
            EnumHttpVersion version_;
            EnumStatusCode statusCode_;
            std::string statusMessage_;
            std::unordered_map<std::string, std::string> headers_;
            std::string responseBody_;
            bool closeConnection_;
        };
    } // namespace http
} // namespace mymuduo

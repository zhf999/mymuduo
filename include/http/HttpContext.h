//
// Created by zhouhf on 2025/5/2.
//

#pragma once

#include <cstddef>
#include <string>

#include "http/HttpRequest.h"
#include "Timestamp.h"
#include "noncopyable.h"

namespace mymuduo {
    class Buffer;

    namespace http {
        class HttpContext : noncopyable {
        private:
            enum class EnumHttpState {
                ExpectRequestLine,
                ExpectRequestHeader,
                ExpectRequestBody,
                Finished
            };

        public:
            HttpContext();
            ~HttpContext() = default;

            bool parse(Buffer *buf, Timestamp receiveTime);
            HttpRequest &request() { return request_; }
            const HttpRequest &request() const { return request_; }

            void reset() {
                state_ = EnumHttpState::ExpectRequestLine;
                parseOk_ = true;
                expectedBodyLength_ = 0;
                request_.clear();
            }

            bool isFinished() const { return state_ == EnumHttpState::Finished; }
            bool parseOk() const { return parseOk_; }

        private:
            bool processRequestLine(const std::string &requestLine, Timestamp receiveTime);
            bool processHeaderLine(const std::string &headerLine);
            bool processBody(Buffer *buf);

            EnumHttpState state_;
            HttpRequest request_;
            bool parseOk_;
            std::size_t expectedBodyLength_;
        };
    } // namespace http
} // namespace mymuduo

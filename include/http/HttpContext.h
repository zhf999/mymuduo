//
// Created by zhouhf on 2025/5/2.
//

#pragma once
#include "http/HttpRequest.h"
#include "Timestamp.h"
#include "noncopyable.h"

namespace mymuduo{
    class Buffer;

    namespace http {
        class HttpContext: noncopyable{
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

            void parse(Buffer *buf, Timestamp receiveTime);
            HttpRequest& request() { return request_; }
            void reset () {
                state_= EnumHttpState::ExpectRequestLine;
                request_.clear();
            }
            bool isFinished() const { return state_== EnumHttpState::Finished; }

        private:
            EnumHttpState state_;
            HttpRequest request_;
        };
    }
} // mymuduo


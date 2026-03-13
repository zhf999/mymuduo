//
// Created by zhouhf on 2025/5/2.
//

#include "http/HttpContext.h"

#include <algorithm>

#include "Buffer.h"

namespace {
    std::string trim(std::string value) {
        const auto first = value.find_first_not_of(" \t");
        if (first == std::string::npos) {
            return "";
        }
        const auto last = value.find_last_not_of(" \t");
        return value.substr(first, last - first + 1);
    }
} // namespace

namespace mymuduo::http {
    HttpContext::HttpContext()
        : state_(EnumHttpState::ExpectRequestLine),
          request_(),
          parseOk_(true),
          expectedBodyLength_(0) {
    }

    bool HttpContext::parse(Buffer *buf, Timestamp receiveTime) {
        while (parseOk_ && !isFinished()) {
            if (state_ == EnumHttpState::ExpectRequestLine) {
                const char *crlf = buf->findSubstr("\r\n");
                if (crlf == nullptr) {
                    break;
                }

                const std::string requestLine = buf->retrieveUntil(crlf);
                buf->retrieveAsString(2);

                if (!processRequestLine(requestLine, receiveTime)) {
                    parseOk_ = false;
                    break;
                }
                state_ = EnumHttpState::ExpectRequestHeader;
            } else if (state_ == EnumHttpState::ExpectRequestHeader) {
                const char *crlf = buf->findSubstr("\r\n");
                if (crlf == nullptr) {
                    break;
                }

                const std::string headerLine = buf->retrieveUntil(crlf);
                buf->retrieveAsString(2);

                if (headerLine.empty()) {
                    expectedBodyLength_ = request_.getContentLength();
                    if (expectedBodyLength_ == 0) {
                        state_ = EnumHttpState::Finished;
                    } else {
                        state_ = EnumHttpState::ExpectRequestBody;
                    }
                    continue;
                }

                if (!processHeaderLine(headerLine)) {
                    parseOk_ = false;
                    break;
                }
            } else if (state_ == EnumHttpState::ExpectRequestBody) {
                if (!processBody(buf)) {
                    break;
                }
            }
        }

        if (!parseOk_) {
            reset();
            parseOk_ = false;
        }

        return parseOk_;
    }

    bool HttpContext::processRequestLine(const std::string &requestLine, Timestamp receiveTime) {
        const std::size_t firstSpace = requestLine.find(' ');
        if (firstSpace == std::string::npos) {
            return false;
        }

        const std::size_t secondSpace = requestLine.find(' ', firstSpace + 1);
        if (secondSpace == std::string::npos) {
            return false;
        }

        const std::string method = requestLine.substr(0, firstSpace);
        const std::string url = requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        const std::string version = requestLine.substr(secondSpace + 1);

        if (method == "GET") {
            request_.setMethod(EnumHttpMethod::GET);
        } else if (method == "POST") {
            request_.setMethod(EnumHttpMethod::POST);
        } else {
            request_.setMethod(EnumHttpMethod::INVALID);
            return false;
        }

        request_.setUrl(url.empty() ? "/" : url);

        if (version == "HTTP/1.0") {
            request_.setVersion(EnumHttpVersion::HTTP_10);
        } else if (version == "HTTP/1.1") {
            request_.setVersion(EnumHttpVersion::HTTP_11);
        } else {
            request_.setVersion(EnumHttpVersion::INVALID);
            return false;
        }

        request_.setReceiveTime(receiveTime);
        return true;
    }

    bool HttpContext::processHeaderLine(const std::string &headerLine) {
        const std::size_t colon = headerLine.find(':');
        if (colon == std::string::npos) {
            return false;
        }

        std::string key = trim(headerLine.substr(0, colon));
        std::string value = trim(headerLine.substr(colon + 1));
        if (key.empty()) {
            return false;
        }

        request_.addHeader(std::move(key), std::move(value));
        return true;
    }

    bool HttpContext::processBody(Buffer *buf) {
        if (buf->readableBytes() < expectedBodyLength_) {
            return false;
        }

        request_.setBody(buf->retrieveAsString(expectedBodyLength_));
        state_ = EnumHttpState::Finished;
        return true;
    }
} // namespace mymuduo::http


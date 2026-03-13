//
// Created by zhouhf on 2025/5/2.
//

#include "http/HttpRequest.h"

#include <algorithm>
#include <cctype>

namespace {
    std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }
} // namespace

namespace mymuduo::http {
    HttpRequest::HttpRequest()
        : method_(EnumHttpMethod::INVALID),
          url_("/"),
          version_(EnumHttpVersion::HTTP_11),
          headers_(),
          body_(),
          receiveTime_() {
    }

    std::string HttpRequest::methodString() const {
        switch (method_) {
            case EnumHttpMethod::GET:
                return "GET";
            case EnumHttpMethod::POST:
                return "POST";
            default:
                return "INVALID";
        }
    }

    std::string HttpRequest::versionString() const {
        switch (version_) {
            case EnumHttpVersion::HTTP_10:
                return "HTTP/1.0";
            case EnumHttpVersion::HTTP_11:
                return "HTTP/1.1";
            default:
                return "HTTP/0.0";
        }
    }

    const std::string &HttpRequest::getHeaderValue(const std::string &key) const {
        auto it = headers_.find(toLower(key));
        if (it != headers_.end()) {
            return it->second;
        }

        static const std::string kEmpty;
        return kEmpty;
    }

    bool HttpRequest::hasHeader(const std::string &key) const {
        return headers_.find(toLower(key)) != headers_.end();
    }

    std::size_t HttpRequest::getContentLength() const {
        auto it = headers_.find("content-length");
        if (it == headers_.end()) {
            return 0;
        }

        try {
            return static_cast<std::size_t>(std::stoul(it->second));
        } catch (...) {
            return 0;
        }
    }

    void HttpRequest::addHeader(std::string key, std::string value) {
        headers_[toLower(std::move(key))] = std::move(value);
    }

    void HttpRequest::clear() {
        method_ = EnumHttpMethod::INVALID;
        url_.clear();
        version_ = EnumHttpVersion::INVALID;
        headers_.clear();
        body_.clear();
        receiveTime_ = Timestamp();
    }
} // namespace mymuduo::http

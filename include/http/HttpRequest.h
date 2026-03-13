//
// Created by zhouhf on 2025/5/2.
//

#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>

#include "Timestamp.h"


namespace mymuduo::http {
    enum class EnumHttpVersion {
        INVALID,
        HTTP_10,
        HTTP_11
    };

    enum class EnumHttpMethod {
        INVALID,
        GET,
        POST
    };

    class HttpContext;

    class HttpRequest {
        friend class HttpContext;

    public:
        HttpRequest();

        EnumHttpMethod getMethod() const { return method_; }
        std::string methodString() const;

        const std::string &getUrl() const { return url_; }

        EnumHttpVersion getVersion() const { return version_; }
        std::string versionString() const;

        const std::string &getHeaderValue(const std::string &key) const;
        bool hasHeader(const std::string &key) const;
        std::size_t getContentLength() const;

        const std::string &getBody() const { return body_; }
        Timestamp receiveTime() const { return receiveTime_; }

        void clear();

    private:
        void setMethod(EnumHttpMethod method) { method_ = method; }
        void setUrl(std::string url) { url_ = std::move(url); }
        void setVersion(EnumHttpVersion version) { version_ = version; }
        void addHeader(std::string key, std::string value);
        void setBody(std::string body) { body_ = std::move(body); }
        void setReceiveTime(Timestamp time) { receiveTime_ = time; }

        EnumHttpMethod method_;
        std::string url_;
        EnumHttpVersion version_;
        std::unordered_map<std::string, std::string> headers_;
        std::string body_;
        Timestamp receiveTime_;
    };
} // namespace mymuduo::http

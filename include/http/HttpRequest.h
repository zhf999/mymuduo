//
// Created by zhouhf on 2025/5/2.
//

#pragma once

#include <string>
#include <unordered_map>

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

    class HttpRequest {
        friend class HttpContext;

    public:

        EnumHttpMethod getMethod() const { return method_; }

        const std::string &getUrl() const { return url_; }

        EnumHttpVersion getVersion() const { return version_; }

        const std::string &getHeaderValue(const std::string &key) const;

        const std::string &getBody() const { return body_; }

        void clear();
    private:
        EnumHttpMethod method_;
        std::string url_;
        EnumHttpVersion version_;
        std::unordered_map<std::string, std::string> headers_;
        std::string body_;
        Timestamp receiveTime_;
    };

} // http
// mymuduo

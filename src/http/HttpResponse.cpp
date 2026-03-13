//
// Created by zhouhf on 2025/5/2.
//

#include "http/HttpResponse.h"

#include <sstream>

#include "Buffer.h"

namespace {
    std::string statusMessageByCode(mymuduo::http::HttpResponse::EnumStatusCode code) {
        using Status = mymuduo::http::HttpResponse::EnumStatusCode;
        switch (code) {
            case Status::k200Ok:
                return "OK";
            case Status::k400BadRequest:
                return "Bad Request";
            case Status::k404NotFound:
                return "Not Found";
            case Status::k500InternalServerError:
                return "Internal Server Error";
            default:
                return "Unknown";
        }
    }

    std::string versionString(mymuduo::http::EnumHttpVersion version) {
        switch (version) {
            case mymuduo::http::EnumHttpVersion::HTTP_10:
                return "HTTP/1.0";
            case mymuduo::http::EnumHttpVersion::HTTP_11:
                return "HTTP/1.1";
            default:
                return "HTTP/1.1";
        }
    }
} // namespace

namespace mymuduo::http {
    HttpResponse::HttpResponse(bool close)
        : version_(EnumHttpVersion::HTTP_11),
          statusCode_(EnumStatusCode::k200Ok),
          statusMessage_("OK"),
          headers_(),
          responseBody_(),
          closeConnection_(close) {
    }

    void HttpResponse::setContentType(const std::string &contentType) {
        headers_["Content-Type"] = contentType;
    }

    void HttpResponse::addHeader(std::string key, std::string value) {
        headers_[std::move(key)] = std::move(value);
    }

    void HttpResponse::writeToBuffer(Buffer *buf) {
        const std::string data = toString();
        buf->append(data.data(), data.size());
    }

    std::string HttpResponse::toString() const {
        std::ostringstream oss;
        const std::string reason = statusMessage_.empty() ? statusMessageByCode(statusCode_) : statusMessage_;

        oss << versionString(version_) << ' ' << static_cast<int>(statusCode_) << ' ' << reason << "\r\n";

        if (closeConnection_) {
            oss << "Connection: close\r\n";
        } else {
            oss << "Connection: Keep-Alive\r\n";
        }

        for (const auto &[key, value] : headers_) {
            oss << key << ": " << value << "\r\n";
        }

        oss << "Content-Length: " << responseBody_.size() << "\r\n";
        oss << "\r\n";
        oss << responseBody_;
        return oss.str();
    }
} // namespace mymuduo::http

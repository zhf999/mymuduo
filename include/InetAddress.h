//
// Created by zhouhf on 2025/4/12.
//
#pragma once

#include <netinet/in.h>
#include <string>

namespace mymuduo
{
    class InetAddress
    {
    public:
        InetAddress() = default;
        explicit InetAddress(const std::string& ip, uint16_t port);
        explicit InetAddress(const sockaddr_in &addr)
                :addr_(addr)
        {};

        [[nodiscard]] std::string toIp() const;
        [[nodiscard]] std::string toIpPort() const;
        [[nodiscard]] uint16_t toPort() const;
        [[nodiscard]] const sockaddr_in* getSockAddr() const {return &addr_;};
        void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }
    private:
        sockaddr_in addr_{};
    };
}


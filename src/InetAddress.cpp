//
// Created by zhouhf on 2025/4/12.
//

#include <InetAddress.h>
#include <cstring>
#include <arpa/inet.h>

namespace mymuduo
{
    InetAddress::InetAddress(const std::string &ip, uint16_t port) {
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    std::string InetAddress::toIp() const {
        char buf[32] = {0};
        inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
        return buf;
    }

    std::string InetAddress::toIpPort() const {
        return toIp() + ":" + std::to_string(toPort());
    }

    uint16_t InetAddress::toPort() const {
        return ntohs(addr_.sin_port);
    }
}


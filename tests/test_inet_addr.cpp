//
// Created by zhouhf on 2025/4/12.
//
#include <iostream>
#include "InetAddress.h"

int main()
{
    mymuduo::InetAddress addr("192.168.11.31",8080);
    std::cout << addr.toIpPort() << std::endl;
    return 0;
}
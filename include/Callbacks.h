//
// Created by zhouhf on 2025/4/19.
//

#pragma once
#include "Timestamp.h"
#include <memory>
#include <functional>

namespace mymuduo{
    class Buffer;
    class TcpConnection;

    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
    using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&,
                                            Buffer*,
                                            Timestamp)>;
    using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&,size_t)>;
}
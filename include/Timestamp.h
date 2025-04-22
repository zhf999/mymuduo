//
// Created by zhouhf on 2025/4/11.
//
#pragma once

#include <iostream>
#include <string>

namespace mymuduo{
    class Timestamp
    {
    public:
        Timestamp();
        explicit Timestamp(int64_t microSecondsSinceEpoch);
        static Timestamp now();
        [[nodiscard]] std::string toString() const;
    private:
        int64_t microSecondsSinceEpoch_;
    };
}

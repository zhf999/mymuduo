//
// Created by zhouhf on 2025/4/11.
//
#pragma once

#include <string>
#include <cstdio>
#include <cstdarg>
#include <sstream>
#include <fstream>
#include <vector>
#include <source_location>
#include <memory>
#include <cstring>
#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

namespace mymuduo{
    class Logger:noncopyable{
    public:
        enum class LogLevel{
            DEBUG,
            INFO,
            ERROR,
            FATAL,
            DISABLED
        };

        static Logger& instance()
        {
            static Logger logger;
            return logger;
        };


        void addFileOutput(LogLevel level, const std::string& file_path)
        {
            auto fs = std::make_shared<std::ofstream >(file_path,std::ios ::out);
            if(!fs->is_open())
                outputs_.emplace_back(level,fs);
//            else Logger::LogError("Can't add file output stream: {}",file_path);
        };

        void setStandardLevel(LogLevel level)
        {
            outputs_[0].logLevel = level;
        }

        void submitLog(LogLevel level,
                       const char* func,
                       int threadId,
                       const char* fmt,
                       ...) {
            char msg[1024];
            va_list args;
            va_start(args, fmt);
            vsnprintf(msg, sizeof(msg), fmt, args);
            va_end(args);

            char formatted[2048];

            snprintf(formatted, sizeof(formatted), "[%s][%s][tid:%d]-[%s]: %s\n",
                     Logger::levelToString(level).c_str(),
                     Timestamp::now().toString().c_str(),
                     threadId,
                     func,
                     msg);

            for(auto &output : outputs_) {
                if(level >= output.logLevel) {
                    *output.os << formatted;
                    output.os->flush();
                }
            }

            if(level == LogLevel::FATAL) {
                exit(EXIT_FAILURE);
            }
        }

    private:
        Logger(){
            outputs_.emplace_back(
                    LogLevel::INFO,
                    std::shared_ptr<std::ostream>(
                            &std::cout,
                            [](std::ostream*){}
                    ));
        }


        static std::string levelToString(Logger::LogLevel level) {
            switch (level) {
                case LogLevel::DEBUG:
                    return "DEBUG";
                case LogLevel::INFO:
                    return "INFO";
                case LogLevel::ERROR:
                    return "ERROR";
                case LogLevel::FATAL:
                    return "FATAL";
                default:
                    return "";
            }
        }

        struct Output{
            LogLevel logLevel;
            std::shared_ptr<std::ostream> os;
        };

        std::vector<Output> outputs_{};
    };

}


#define LOG_DEBUG(fmt, ...) \
    mymuduo::Logger::instance().submitLog(mymuduo::Logger::LogLevel::DEBUG,  \
    std::source_location::current().function_name(), mymuduo::CurrentThread::tid(),  \
    fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    mymuduo::Logger::instance().submitLog(mymuduo::Logger::LogLevel::INFO,   \
    std::source_location::current().function_name(), mymuduo::CurrentThread::tid(),  \
    fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    mymuduo::Logger::instance().submitLog(mymuduo::Logger::LogLevel::ERROR,  \
    std::source_location::current().function_name(), mymuduo::CurrentThread::tid(),   \
    fmt, ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...) \
    mymuduo::Logger::instance().submitLog(mymuduo::Logger::LogLevel::FATAL,  \
    std::source_location::current().function_name(), mymuduo::CurrentThread::tid(),   \
    fmt, ##__VA_ARGS__)

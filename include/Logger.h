//
// Created by zhouhf on 2025/4/11.
//
#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <format>
#include <source_location>
#include <memory>
#include "noncopyable.h"
#include "Timestamp.h"

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

        template<typename... Args>
        static void LogDebug(std::format_string<Args...> fmt, Args&&... args)
        {
            instance().submitLog(LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
        };

        template <typename... Args>
        static void LogInfo(std::format_string<Args...> fmt, Args&&... args)
        {
            instance().submitLog(LogLevel::INFO, fmt, std::forward<Args>(args)...);
        };

        template <typename... Args>
        static void LogError(std::format_string<Args...> fmt, Args&&... args)
        {
            instance().submitLog(LogLevel::ERROR, fmt, std::forward<Args>(args)...);
        };

        template <typename... Args>
        static void LogFatal(std::format_string<Args...> fmt, Args&&... args)
        {
            instance().submitLog(LogLevel::FATAL, fmt, std::forward<Args>(args)...);
            exit(0);
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

    private:
        Logger(){
            outputs_.emplace_back(
                    LogLevel::INFO,
                    std::shared_ptr<std::ostream>(
                            &std::cout,
                            [](std::ostream*){}
                    ));
        }

        template<typename... Args>
        void submitLog(LogLevel level,
                       std::format_string<Args...> fmt, Args&&... args)
        {
            std::string msg = std::format("[{}]-[{}]: {}\n",
                                          levelToString(level),
                                          Timestamp::now().toString(),
                                          std::format(fmt,std::forward<Args>(args)...));
            for(auto &output:outputs_)
            {
                if(level>=output.logLevel)
                {
                    *output.os << msg;
                    output.os->flush();
                }
            }
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

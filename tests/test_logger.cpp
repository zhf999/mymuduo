//
// Created by zhouhf on 2025/4/11.
//

#include "Logger.h"
using namespace mymuduo;

int main()
{
    Logger::instance().addFileOutput(Logger::LogLevel::DEBUG, "debuglog.log");
    Logger::LogDebug("this is a debug");
    Logger::LogInfo("this is a info");
    Logger::LogError("this is a error");
    Logger::LogFatal("this is a fatal");
    return 0;
}
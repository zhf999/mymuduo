//
// Created by zhouhf on 2025/4/11.
//

#include "Logger.h"

using namespace mymuduo;

int main()
{
    Logger::instance().addFileOutput(Logger::LogLevel::DEBUG, "debuglog.log");
    LOG_DEBUG("this is a debug %d",12412);
    LOG_INFO("this is a info %d",123456);
    LOG_ERROR("this is a error");
    LOG_FATAL("this is a fatal");
    return 0;
}
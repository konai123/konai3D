//
// Created by korona on 2021-06-19.
//

#include "src/engine/core/logger.hpp"

#ifndef MACROS_H
#define MACROS_H

#define APP_LOG_FILE_PATH      "logs/app.txt"
#define APP_LOG_DEBUG(...)      _ENGINE::Logger::Instance(APP_LOG_FILE_PATH)->debug(__VA_ARGS__)
#define APP_LOG_INFO(...)       _ENGINE::Logger::Instance(APP_LOG_FILE_PATH)->info(__VA_ARGS__)
#define APP_LOG_WARNING(...)    _ENGINE::Logger::Instance(APP_LOG_FILE_PATH)->warn(__VA_ARGS__)
#define APP_LOG_ERROR(...)      _ENGINE::Logger::Instance(APP_LOG_FILE_PATH)->error(__VA_ARGS__)

#if defined(DEBUG) || defined(_DEBUG)
#define AppAssert(x) do {                                           \
    if (!(x)) {                                                     \
        APP_LOG_ERROR("[App Assert] on {} - {}::{}",      \
        __FILE__, __FUNCTION__, __LINE__);                          \
        abort();                                                    \
    }                                                               \
}while(0)

#else
#define AppAssert(x) void(x)
#endif /*DEBUG*/
#endif //MACROS_H

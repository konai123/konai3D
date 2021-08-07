//
// Created by korona on 2021-06-14.
//

#pragma once

#include "src/engine/core/logger.hpp"

#define ReturnFalseHRFailed(hret) do {                      \
    if (FAILED(hret)) {                                     \
        return false;                                       \
    }                                                       \
}while(0)

#define ReturnNullHRFailed(hret) do {                       \
    if (FAILED(hret)) {                                     \
        return nullptr;                                     \
    }                                                       \
}while(0)

#define ThrowHRFailed(hret) do {                            \
    if (FAILED(hret)) {                                     \
        throw _ENGINE::ComException(hret); \
    }                                                       \
}while(0)

#define GRAPHICS_LOG_FILE_PATH      "logs/graphics.txt"
#define GRAPHICS_LOG_DEBUG(...)      _ENGINE::Logger::Instance(GRAPHICS_LOG_FILE_PATH)->debug(__VA_ARGS__)
#define GRAPHICS_LOG_INFO(...)       _ENGINE::Logger::Instance(GRAPHICS_LOG_FILE_PATH)->info(__VA_ARGS__)
#define GRAPHICS_LOG_WARNING(...)    _ENGINE::Logger::Instance(GRAPHICS_LOG_FILE_PATH)->warn(__VA_ARGS__)
#define GRAPHICS_LOG_ERROR(...)      _ENGINE::Logger::Instance(GRAPHICS_LOG_FILE_PATH)->error(__VA_ARGS__)

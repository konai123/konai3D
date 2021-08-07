//
// Created by korona on 2021-06-05.
//

#pragma once

#include "src/engine/core/logger.hpp"
#include "src/engine/core/com_exception.hpp"

#define CORE_LOG_FILE_PATH      "logs/core.txt"
#define CORE_LOG_DEBUG(...)      _ENGINE::Logger::Instance(CORE_LOG_FILE_PATH)->debug(__VA_ARGS__)
#define CORE_LOG_INFO(...)       _ENGINE::Logger::Instance(CORE_LOG_FILE_PATH)->info(__VA_ARGS__)
#define CORE_LOG_WARNING(...)    _ENGINE::Logger::Instance(CORE_LOG_FILE_PATH)->warn(__VA_ARGS__)
#define CORE_LOG_ERROR(...)      _ENGINE::Logger::Instance(CORE_LOG_FILE_PATH)->error(__VA_ARGS__)

#if defined(DEBUG) || defined(_DEBUG)
#define EngineAssert(x) do {                                          \
    if (!(x)) {                                                     \
        CORE_LOG_ERROR("[Core Assert] on {} - {}::{}",              \
        __FILE__, __FUNCTION__, __LINE__);                          \
        abort();                                                    \
    }                                                               \
}while(0)
#else
#define EngineAssert(x) void(x)
#endif /*DEBUG*/

#define DECLARE_CLASS_AS_INTERFACE(ClassName)                                \
   public :                                                                  \
      virtual ~ClassName() {}                                                \
   protected :                                                               \
      ClassName() {}                                                         \
      ClassName(const ClassName & ) {}                                       \
      ClassName & operator = (const ClassName & ) { return *this ; }         \
      ClassName(ClassName && ) noexcept {}                                   \
      ClassName & operator = (ClassName && ) noexcept { return *this ; }

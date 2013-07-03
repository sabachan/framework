#ifndef Core_Log_H
#define Core_Log_H

#include "Config.h"
#include <string>

namespace sg {
namespace logging {
//=============================================================================
void Debug(char const* msg);
void Info(char const* msg);
void Warning(char const* msg);
void Error(char const* msg);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Debug(std::string const& msg);
void Info(std::string const& msg);
void Warning(std::string const& msg);
void Error(std::string const& msg);
//void Progress(char const* name, size_t current, size_t size);
//class Scope
//{
//public:
//    Scope(char const* name);
//};
//=============================================================================
}
}

#if SG_ENABLE_LOG
#define SG_CODE_FOR_LOG(expr) expr
#define SG_LOG_DEBUG(msg) sg::logging::Debug(msg)
#define SG_LOG_INFO(msg)  sg::logging::Info(msg)
#define SG_LOG_WARNING(msg) sg::logging::Warning(msg)
#define SG_LOG_ERROR(msg)  sg::logging::Error(msg)
#else
#define SG_CODE_FOR_LOG(expr)
#define SG_LOG_DEBUG(msg) ((void)0)
#define SG_LOG_INFO(msg) ((void)0)
#define SG_LOG_WARNING(msg) ((void)0)
#define SG_LOG_ERROR(msg) ((void)0)
#endif

#endif

#ifndef Core_Log_H
#define Core_Log_H

#include "Config.h"
#include <string>

namespace sg {
namespace logging {
//=============================================================================
enum class Type { Debug, Info, Warning, Error };
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
typedef void LogFct(char const* domain, Type type, char const* msg);
void DefaultLogCallback(char const* domain, Type type, char const* msg);
LogFct* GetAndSetLogCallback(LogFct* f);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Log(char const* domain, Type type, char const* msg);
void Log(char const* domain, Type type, std::string const& msg);
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

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
#define SG_LOG_DEFAULT_DEBUG(msg)   ::sg::logging::Log(nullptr, ::sg::logging::Type::Debug,   msg)
#define SG_LOG_DEFAULT_INFO(msg)    ::sg::logging::Log(nullptr, ::sg::logging::Type::Info,    msg)
#define SG_LOG_DEFAULT_WARNING(msg) ::sg::logging::Log(nullptr, ::sg::logging::Type::Warning, msg)
#define SG_LOG_DEFAULT_ERROR(msg)   ::sg::logging::Log(nullptr, ::sg::logging::Type::Error,   msg)
#define SG_LOG_DEBUG(domain, msg)   ::sg::logging::Log(domain, ::sg::logging::Type::Debug,   msg)
#define SG_LOG_INFO(domain, msg)    ::sg::logging::Log(domain, ::sg::logging::Type::Info,    msg)
#define SG_LOG_WARNING(domain, msg) ::sg::logging::Log(domain, ::sg::logging::Type::Warning, msg)
#define SG_LOG_ERROR(domain, msg)   ::sg::logging::Log(domain, ::sg::logging::Type::Error,   msg)
#else
#define SG_CODE_FOR_LOG(expr)
#define SG_LOG_DEFAULT_DEBUG(domain, msg)   ((void)0)
#define SG_LOG_DEFAULT_INFO(domain, msg)    ((void)0)
#define SG_LOG_DEFAULT_WARNING(domain, msg) ((void)0)
#define SG_LOG_DEFAULT_ERROR(domain, msg)   ((void)0)
#define SG_LOG_DEBUG(domain, msg)   ((void)0)
#define SG_LOG_INFO(domain, msg)    ((void)0)
#define SG_LOG_WARNING(domain, msg) ((void)0)
#define SG_LOG_ERROR(domain, msg)   ((void)0)
#endif

#endif

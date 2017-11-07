#include "stdafx.h"

#include "Log.h"

#include "Assert.h"
#include "Config.h"
#include "Platform.h"
#include "StringFormat.h"

#if SG_PLATFORM_IS_WIN
#include <Core/WindowsH.h>
#endif

namespace sg {
namespace logging {
//=============================================================================
namespace {
LogFct* s_LogCallback = DefaultLogCallback;
}
//=============================================================================
void DefaultLogCallback(char const* domain, Type type, char const* msg)
{
#if SG_PLATFORM_IS_WIN
    if(nullptr != domain)
        OutputDebugStringA(Format("[%0] ", domain).c_str());
    switch(type)
    {
    case Type::Debug:
    case Type::Info:
        break;
    case Type::Warning:
        OutputDebugStringA("[WARNING] ");
        break;
    case Type::Error:
        OutputDebugStringA("[ERROR] ");
        break;
    default:
        SG_ASSERT_NOT_REACHED();
    }
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
#else
#error "todo"
#endif
}
//=============================================================================
LogFct* GetAndSetLogCallback(LogFct* f)
{
    LogFct* r = s_LogCallback;
    s_LogCallback = f;
    return r;
}
//=============================================================================
void Log(char const* domain, Type type, char const* msg)
{
    if(nullptr != s_LogCallback)
        s_LogCallback(domain, type, msg);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Log(char const* domain, Type type, std::string const& msg)
{
    Log(domain, type, msg.c_str());
}
//=============================================================================
}
}

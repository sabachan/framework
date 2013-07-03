#include "stdafx.h"

#include "Log.h"

#include "Config.h"
#include "Platform.h"

#if SG_PLATFORM_IS_WIN
#include <Core/WindowsH.h>
#endif

namespace sg {
namespace logging {
//=============================================================================
namespace {
    void Log_Impl(char const* msg) // TODO: add type
    {
#if SG_PLATFORM_IS_WIN
        OutputDebugStringA(msg);
        OutputDebugStringA("\n");
#else
#error "todo"
#endif
    }
}
//=============================================================================
void Debug(char const* msg)
{
    Log_Impl(msg);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Info(char const* msg)
{
    Log_Impl(msg);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Warning(char const* msg)
{
    Log_Impl(msg);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Error(char const* msg)
{
    Log_Impl(msg);
}
//=============================================================================
void Debug(std::string const& msg)
{
    Log_Impl(msg.c_str());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Info(std::string const& msg)
{
    Log_Impl(msg.c_str());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Warning(std::string const& msg)
{
    Log_Impl(msg.c_str());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Error(std::string const& msg)
{
    Log_Impl(msg.c_str());
}
//=============================================================================
}
}

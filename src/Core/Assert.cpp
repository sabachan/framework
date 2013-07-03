#include "stdafx.h"

#include "Assert.h"

#include "IntTypes.h"
#include "Log.h"
#include "WindowsH.h"
#include <sstream>
#include <unordered_set>

namespace sg {
//=============================================================================
namespace {
    size_t PackFileAndLine_CanCollide(char const* iFile, size_t iLine)
    {
        static const size_t rscount = sizeof(size_t)*8 * 3/5;
        return (ptrdiff_t)iFile ^ (iLine << rscount);
    }
}
//=============================================================================
bool HandleAssertFailedReturnMustBreak(char const* iExpr,
                                       char const* iMsg,
                                       char const* iFile,
                                       size_t iLine,
                                       char const* iFctName)
{
    std::ostringstream oss;
    oss << "Assert FAILED :" << std::endl;
    oss << "    (" << iExpr << ") is false" << std::endl;
    oss << "in " << iFctName << " (" << iFile << ":" << iLine << ")." << std::endl;
    if(nullptr != iMsg)
        oss << std::endl << iMsg << std::endl;
    SG_LOG_ERROR(oss.str().c_str());

    static std::unordered_set<size_t> toIgnore;
    if(!toIgnore.empty())
    {
        auto f = toIgnore.find(PackFileAndLine_CanCollide(iFile, iLine));
        if(toIgnore.end() != f)
            return false;
    }

    bool enableBreakpoint = true;
    bool ignoreThisAssert = false;

#if SG_PLATFORM_IS_WIN
    bool const isDebuggerPresent = IsDebuggerPresent() == TRUE;
#else
#error "todo"
#endif
    if(!isDebuggerPresent) // disabled with debugger, as there is already a breakpoint with its own window
    {
        std::ostringstream oss2;
        oss2 << oss.str() << std::endl;
        oss2 << "(press abort to exit program, retry to continue, ignore to mute this assert)" << std::endl;
        int ret = MessageBox(
            NULL,
            oss2.str().c_str(),
            "Assert failed!",
            MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND | MB_DEFBUTTON2
            );
        switch(ret)
        {
        case IDABORT:
#if !SG_ENABLE_ASSERT
            exit(1);
#endif
            enableBreakpoint = true;
            ignoreThisAssert = false;
            break;
        case IDIGNORE:
            enableBreakpoint = false;
            ignoreThisAssert = true;
            break;
        case IDRETRY:
            enableBreakpoint = false;
            ignoreThisAssert = false;
            break;
        default:
            SG_ASSERT_NOT_REACHED();
            return false;
        }
    }

    if(ignoreThisAssert)
    {
        toIgnore.insert(PackFileAndLine_CanCollide(iFile, iLine));
    }

    return enableBreakpoint;
}
//=============================================================================
}

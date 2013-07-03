#include "stdafx.h"

#include "WinUtils.h"

#include "Assert.h"
#include "Cast.h"
#include "Log.h"
#include "StringUtils.h"
#include <sstream>

#if SG_PLATFORM_IS_WIN
#include <Core/WindowsH.h>
#endif

namespace sg {
namespace winutils {
//=============================================================================
bool DoesFileOrDirectoryExists(std::string const& iPath)
{
    HANDLE handle = CreateFileW(
        ConvertUTF8ToUCS2(iPath).c_str(),
        0, // 0: can query metadata
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, // Security Attributes
        OPEN_EXISTING, // Creation Disposition
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, // Flags and Attributes
        NULL // Template File
        );
    if(INVALID_HANDLE_VALUE == handle)
        return false;

    CloseHandle(handle);
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
u64 GetFileModificationTimestamp(std::string const& iPath)
{
    HANDLE handle = CreateFileW(
        ConvertUTF8ToUCS2(iPath).c_str(),
        0, // 0: can query metadata
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, // Security Attributes
        OPEN_EXISTING, // Creation Disposition
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, // Flags and Attributes
        NULL // Template File
        );
    SG_ASSERT(INVALID_HANDLE_VALUE != handle);
    if(INVALID_HANDLE_VALUE == handle)
        return 0;

    BY_HANDLE_FILE_INFORMATION info;
    BOOL rc = GetFileInformationByHandle(
        handle,
        &info
        );
    SG_ASSERT_AND_UNUSED(rc);

    ULARGE_INTEGER ts;
    ts.LowPart = info.ftLastWriteTime.dwLowDateTime;
    ts.HighPart = info.ftLastWriteTime.dwHighDateTime;

    CloseHandle(handle);
    return num_cast<u64>(ts.QuadPart);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool WriteFileOverwriteIFNROK(std::string const& iPath, u8 const* iData, size_t iSize)
{
    HANDLE handle = CreateFileW(
        ConvertUTF8ToUCS2(iPath).c_str(),
        GENERIC_WRITE,
        0,
        NULL, // Security Attributes
        CREATE_ALWAYS, // Creation Disposition
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, // Flags and Attributes
        NULL // Template File
        );
    if(INVALID_HANDLE_VALUE == handle)
        return false;

    DWORD writtenByteCount = 0;
    BOOL rc = WriteFile(
        handle,
        iData,
        checked_numcastable(iSize),
        &writtenByteCount,
        NULL
        );
    SG_ASSERT_AND_UNUSED(rc);

    CloseHandle(handle);
    return true;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GetWinWorkingDir(std::string& oPath)
{
    static const size_t N = 2048;
    wchar_t buffer[N];
    DWORD r = GetCurrentDirectoryW(N, buffer);
    if(r >= N)
    {
        std::wstring str;
        str.resize(r-1);
        DWORD r2 = GetCurrentDirectoryW(r, &str[0]);
        SG_ASSERT_AND_UNUSED(r2 <= r);
        SG_ASSERT(0 != r2);
        oPath = ConvertUCS2ToUTF8(str);
    }
    else
    {
        SG_ASSERT(0 != r);
        oPath = ConvertUCS2ToUTF8(buffer);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetWinWorkingDir(std::string const& iPath)
{
    BOOL r = SetCurrentDirectoryW(ConvertUTF8ToUCS2(iPath).c_str());
    SG_ASSERT_AND_UNUSED(r);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string GetWinLastError()
{
    DWORD lastErr = GetLastError();
    SG_ASSERT(0 != lastErr);
    char buffer[1024];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL,
                     lastErr,
                     MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                     buffer,
                     SG_ARRAYSIZE(buffer),
                     NULL);
    return buffer;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void PrintWinLastError(char const* iFunctionName)
{
    DWORD lastErr = GetLastError();
    SG_ASSERT(0 != lastErr);
    char buffer[1024];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL,
                     lastErr,
                     MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                     buffer,
                     SG_ARRAYSIZE(buffer),
                     NULL);
    std::ostringstream oss;
    oss << iFunctionName << " failed with error: " << std::endl;
    oss << "    " << buffer << std::endl;
    SG_LOG_ERROR(oss.str().c_str());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ShowModalErrorReturnRetry(char const* iErrorTitle, char const* iErrorMsg)
{
    std::ostringstream oss;
    std::string errorMsg(iErrorMsg);
    if(errorMsg.size() > 1500)
        oss << errorMsg.substr(0, 1500) << std::endl << "[...]" << std::endl << std::endl;
    else
        oss << errorMsg << std::endl << std::endl;
    int ret = MessageBox(
        NULL,
        oss.str().c_str(),
        iErrorTitle,
        MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND | MB_DEFBUTTON2
        );
    switch(ret)
    {
    case IDABORT:
        exit(1);
        return false;
    case IDIGNORE:
        return false;
    case IDRETRY:
        return true;
    default:
        SG_ASSERT_NOT_REACHED();
        return false;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShowModalMessageOK(char const* iErrorTitle, char const* iErrorMsg)
{
    std::ostringstream oss;
    std::string errorMsg(iErrorMsg);
    if(errorMsg.size() > 1500)
        oss << errorMsg.substr(0, 1500) << std::endl << "[...]" << std::endl << std::endl;
    else
        oss << errorMsg << std::endl << std::endl;
    int const ret = MessageBox(
        NULL,
        oss.str().c_str(),
        iErrorTitle,
        MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST | MB_SETFOREGROUND | MB_DEFBUTTON2
        );
    SG_UNUSED(ret);
    SG_ASSERT(0 != ret);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GetUniqueSystemFilePath_AssumeExists(std::string& oPath, std::string const& iPath)
{
    wchar_t buffer[2 * 1024];
    std::wstring const pathstr = ConvertUTF8ToUCS2(iPath);
    DWORD const length = GetLongPathNameW(
        pathstr.c_str(),
        buffer,
        SG_ARRAYSIZE(buffer));
    SG_ASSERT(0 < length);
    if(length < SG_ARRAYSIZE(buffer))
        oPath = ConvertUCS2ToUTF8(std::wstring(buffer, length));
    else
    {
        std::wstring longstr;
        longstr.resize(length-1);
        DWORD const length2 = GetLongPathNameW(
            pathstr.c_str(),
            &longstr[0],
            checked_numcastable(longstr.size()));
        SG_ASSERT(length2 == longstr.length());
        oPath = ConvertUCS2ToUTF8(longstr);
    }
}
//=============================================================================
}
}

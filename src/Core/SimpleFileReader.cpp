#include "stdafx.h"

#include "Assert.h"
#include "Cast.h"
#include "Log.h"
#include "Platform.h"
#include "SimpleFileReader.h"
#include "StringUtils.h"
#include "WinUtils.h"
#include <sstream>

#if SG_PLATFORM_IS_WIN
#include <Core/WindowsH.h>
#endif

namespace sg {
//=============================================================================
SimpleFileReader::SimpleFileReader(FilePath const& iFilename)
    : m_data(nullptr)
    , m_size(0)
    , m_isValid(true)
{
    HANDLE handle = CreateFileW(
        ConvertUTF8ToUCS2(iFilename.GetSystemFilePath()).c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, // Security Attributes
        OPEN_EXISTING, // Creation Disposition
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, // Flags and Attributes
        NULL // Template File
        );
    if(INVALID_HANDLE_VALUE == handle)
    {
        std::ostringstream oss;
        oss << "can't open file: " << iFilename.GetPrintableString();
        SG_LOG_ERROR(oss.str().c_str());
        m_isValid = false;
        return;
    }

    LARGE_INTEGER filesize;
    BOOL rc = GetFileSizeEx(
        handle,
        &filesize
        );
    if(!rc)
    {
        std::ostringstream oss;
        oss << "GetFileSizeEx(" << iFilename.GetPrintableString() << ")";
        winutils::PrintWinLastError(oss.str().c_str());
        m_isValid = false;
    }

    SG_ASSERT(filesize.QuadPart != 0);

    m_size = checked_numcastable(filesize.QuadPart);
    m_data = malloc(m_size);

    DWORD byteCountToRead = num_cast<DWORD>(m_size);
    DWORD readByteCount = 0;
    rc = ReadFile(
        handle,
        m_data,
        byteCountToRead,
        &readByteCount,
        NULL
        );
    if(!rc)
    {
        free(m_data);
        m_data = nullptr;
        m_size = 0;

        std::ostringstream oss;
        oss << "ReadFile(" << iFilename.GetPrintableString() << ")";
        winutils::PrintWinLastError(oss.str().c_str());
        m_isValid = false;
    }

    CloseHandle(handle);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SimpleFileReader::~SimpleFileReader()
{
    free(m_data);
}
//=============================================================================
}

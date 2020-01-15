#include "stdafx.h"

#include "pnm.h"

#include <sstream>
#include <type_traits>
#include <Core/Assert.h>
#include <Core/Cast.h>
#include <Core/StringFormat.h>
#include <Core/StringUtils.h>
#include <Core/WindowsH.h>
#include <Core/WinUtils.h>

namespace sg {
namespace pnm {
//=============================================================================
namespace {
bool WritePNMROK(FilePath const& filename,
                 size_t iType,
                 u8 const* iData,
                 size_t iDataSize,
                 size_t iWidth,
                 size_t iHeight,
                 size_t iStrideInBytes)
{
    SG_UNUSED(iDataSize);
    SG_ASSERT(5 <= iType && iType <= 6);
    size_t const pixelSizeInByte = iType == 5 ? 1 : iType == 6 ? 3 : -1;
    SG_ASSERT((iHeight-1) * iStrideInBytes + iWidth * pixelSizeInByte <= iDataSize);

    std::ostringstream oss;
    oss << "P" << iType << std::endl;
    oss << iWidth << " " << iHeight << std::endl;
    oss << "255" << std::endl;
    std::string header = oss.str();

#if SG_PLATFORM_IS_WIN
    winutils::CreateDirectoryPathIFN(filename.ParentDirectory().GetSystemFilePath());

    HANDLE handle = CreateFileW(
        ConvertUTF8ToUCS2(filename.GetSystemFilePath()).c_str(),
        GENERIC_WRITE,
        0,
        NULL, // Security Attributes
        CREATE_ALWAYS, // Creation Disposition
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, // Flags and Attributes
        NULL // Template File
        );
    SG_ASSERT(INVALID_HANDLE_VALUE != handle);
    if(INVALID_HANDLE_VALUE == handle)
    {
        winutils::PrintWinLastError(Format("CreateFileW(%0)", filename.GetSystemFilePath()).c_str());
        return false;
    }

    DWORD writtenByteCount = 0;
    BOOL rc = WriteFile(
        handle,
        header.data(),
        checked_numcastable(header.size()),
        &writtenByteCount,
        NULL
        );
    SG_ASSERT_AND_UNUSED(rc);

    for(size_t y = 0; y < iHeight; ++y)
    {
        u8 const* row = iData + y * iStrideInBytes;
        writtenByteCount = 0;
        rc = WriteFile(
            handle,
            row,
            checked_numcastable(iWidth * pixelSizeInByte),
            &writtenByteCount,
            NULL
            );
        SG_ASSERT_AND_UNUSED(rc);
    }

    CloseHandle(handle);
#else
#error "TODO"
#endif
    return true;
}
}
//=============================================================================
bool WriteGrayROK(FilePath const& filename,
                  u8 const* iData,
                  size_t iDataSize,
                  size_t iWidth,
                  size_t iHeight,
                  size_t iStrideInBytes)
{
    return WritePNMROK(filename, 5, iData, iDataSize, iWidth, iHeight, iStrideInBytes);
}
//=============================================================================
bool WriteRGBROK(FilePath const& filename,
                 u8 const* iData,
                 size_t iDataSize,
                 size_t iWidth,
                 size_t iHeight,
                 size_t iStrideInBytes)
{
    return WritePNMROK(filename, 6, iData, iDataSize, iWidth, iHeight, iStrideInBytes);
}
//=============================================================================
}
}

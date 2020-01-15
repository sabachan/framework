#ifndef Core_WinUtils_H
#define Core_WinUtils_H

#include "IntTypes.h"
#include "Platform.h"
#if !SG_PLATFORM_IS_WIN
#error "Windows only !"
#endif

#include <string>

namespace sg {
namespace winutils {
//=============================================================================
bool DoesFileOrDirectoryExists(std::string const& iPath);
bool IsDirectory(std::string const& iPath);
void CreateDirectory(std::string const& iPath);
void CreateDirectoryPathIFN(std::string const& iPath);
u64 GetFileModificationTimestamp(std::string const& iPath);
bool WriteFileOverwriteIFNROK(std::string const& iPath, u8 const* iData, size_t iSize);
void GetWinWorkingDir(std::string& oPath);
void SetWinWorkingDir(std::string const& iPath);
std::string GetWinLastError();
void PrintWinLastError(char const* iFunctionName);
bool ShowModalErrorReturnRetry(char const* iErrorTitle, char const* iErrorMsg);
void ShowModalMessageOK(char const* iErrorTitle, char const* iErrorMsg);
void GetUniqueSystemFilePath_AssumeExists(std::string& oPath, std::string const& iPath);
//=============================================================================
}
}

#endif

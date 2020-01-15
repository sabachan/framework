#ifndef System_Clipboard_H
#define System_Clipboard_H

#include <string>

namespace sg {
namespace system {
//=============================================================================
void CopyTextToClipboard(std::wstring const iText);
void GetTextInClipboard(std::wstring& oText);
//=============================================================================
}
}

#endif

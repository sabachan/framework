#include "stdafx.h"

#include "Clipboard.h"

#include <Core/Assert.h>
#include <Core/Platform.h>
#include <Core/StringUtils.h>
#include <Core/WindowsH.h>
#include <Core/WinUtils.h>

namespace sg {
namespace system {
//=============================================================================
void CopyTextToClipboard(std::wstring const iText)
{
#if SG_PLATFORM_IS_WIN
    BOOL rc = OpenClipboard(nullptr);
    SG_ASSERT(rc);
    if(!rc)
    {
        winutils::PrintWinLastError("OpenClipboard");
        return;
    }
    rc = EmptyClipboard();
    SG_ASSERT(rc);
    if(!rc)
    {
        winutils::PrintWinLastError("EmptyClipboard");
        rc = CloseClipboard();
        SG_ASSERT(rc);
        return;
    }

    std::string const utf8Text = ConvertUCS2ToUTF8(iText);

    // Allocate a global memory object for the text.
    HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, (iText.length() + 1) * sizeof(wchar_t));
    SG_ASSERT(nullptr != hData);
    if(nullptr == hData)
    { 
        CloseClipboard();
        return;
    }

    // Lock the handle and copy the text to the buffer.
    wchar_t* strCopy = static_cast<wchar_t*>(GlobalLock(hData));
    memcpy(strCopy, iText.data(), (iText.length() + 1) * sizeof(wchar_t));
    SG_ASSERT(0 == strCopy[iText.length()]);
    GlobalUnlock(hData);

    // Place the handle on the clipboard. 
    HANDLE rh = SetClipboardData(CF_UNICODETEXT, hData);
    if(nullptr == rh)
    {
        winutils::PrintWinLastError("SetClipboardData");
    }

    rc = CloseClipboard();
    SG_ASSERT(rc);
#else
    SG_ASSERT_NOT_IMPLEMENTED();
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GetTextInClipboard(std::wstring& oText)
{
#if SG_PLATFORM_IS_WIN
    BOOL rc = OpenClipboard(nullptr);
    SG_ASSERT(rc);
    if(!rc)
    {
        winutils::PrintWinLastError("OpenClipboard");
        return;
    }

    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    SG_ASSERT(nullptr != hData);
    if(nullptr == hData)
    { 
        CloseClipboard();
        return;
    }

    // Lock the handle to get the actual text pointer
    wchar_t* text = static_cast<wchar_t*>(GlobalLock(hData));
    if(text == nullptr)
    {
        CloseClipboard();
        return;
    }

    oText = text;

    GlobalUnlock(hData);

    rc = CloseClipboard();
    SG_ASSERT(rc);
#else
    SG_ASSERT_NOT_IMPLEMENTED();
#endif
}
//=============================================================================
}
}

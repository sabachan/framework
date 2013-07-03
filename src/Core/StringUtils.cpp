#include "stdafx.h"

#include "StringUtils.h"

#include <string>
#include <codecvt>

namespace sg {
//=============================================================================
std::wstring ConvertUTF8ToUCS2(std::string const& s)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring ucs2_string = converter.from_bytes(s);
    return ucs2_string;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
std::string ConvertUCS2ToUTF8(std::wstring const& s)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
    std::string utf8_string = convert.to_bytes(s);
    return utf8_string;
}
//=============================================================================
}

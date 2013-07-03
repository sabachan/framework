#ifndef Core_StringUtils_H
#define Core_StringUtils_H

#include "Config.h"
#include <string>

namespace sg {
//=============================================================================
std::wstring ConvertUTF8ToUCS2(std::string const& s);
std::string ConvertUCS2ToUTF8(std::wstring const& s);
//=============================================================================
}

#endif

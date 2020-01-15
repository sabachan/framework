#ifndef Core_WindowsH_H
#define Core_WindowsH_H

#if !SG_PLATFORM_IS_WIN
#error "Do not include this file if not on Windows."
#endif

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#undef near
#undef far

#endif

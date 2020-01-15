#ifndef Rendering_WTF_IncludeDxgiFormat_H
#define Rendering_WTF_IncludeDxgiFormat_H

#if !SG_PLATFORM_IS_WIN
#error "Do not include this file if not on Windows."
#endif

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define near
#define far
#include <dxgiformat.h>
#undef near
#undef far

#endif

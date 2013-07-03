// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#ifndef DemoApplication_stdafx_H
#define DemoApplication_stdafx_H

#include <Core/stdafx.h>
#include <FileFormats/stdafx.h>
#include <Image/stdafx.h>
#include <Math/stdafx.h>
#include <ObjectScript/stdafx.h>
#include <Reflection/stdafx.h>
#include <RenderEngine/stdafx.h>
#include <Rendering/stdafx.h>

//=============================================================================
#include <Core/Platform.h>
#if SG_PLATFORM_IS_WIN
#include "targetver.h"
#include <tchar.h>
#endif
//=============================================================================

#endif

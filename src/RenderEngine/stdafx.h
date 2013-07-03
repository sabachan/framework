#pragma once
#ifndef RenderEngine_stdafx_H
#define RenderEngine_stdafx_H

#include <Core/stdafx.h>
#include <FileFormats/stdafx.h>
#include <Image/stdafx.h>
#include <Math/stdafx.h>
#include <Reflection/stdafx.h>
#include <Rendering/stdafx.h>

#if SG_USE_STDAFX
//=============================================================================
#include <cstring>

#if SG_PLATFORM_IS_WIN
#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <d3d11shader.h>
#include <d3dcommon.h>
#include <D3Dcompiler.h>
#include <dxgi.h>
#endif
//=============================================================================
#endif

#endif


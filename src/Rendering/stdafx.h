#pragma once
#ifndef Rendering_stdafx_H
#define Rendering_stdafx_H

#include <Core/stdafx.h>
#include <FileFormats/stdafx.h>
#include <Image/stdafx.h>
#include <Math/stdafx.h>

#if SG_USE_STDAFX
//=============================================================================
#include <cstring>

#if SG_PLATFORM_IS_WIN
#include "WTF/IncludeD3D11.h"
#include "WTF/IncludeD3D11SDKLayers.h"
#include "WTF/IncludeD3D11Shader.h"
#include "WTF/IncludeD3DCommon.h"
#include "WTF/IncludeD3DCompiler.h"
#include "WTF/IncludeDxgi.h"
#include "WTF/IncludeDxgiFormat.h"
#endif

//=============================================================================
#if SG_ENABLE_PROJECT_INCLUDES_IN_STDAFX
#include "Color.h"
#include "IRenderTarget.h"
#include "IShaderResource.h"
#include "Material.h"
#include "RenderBatch.h"
#include "RenderBatchDico.h"
#include "RenderBatchSet.h"
#include "RenderDevice.h"
#include "RenderState.h"
#include "RenderStateDico.h"
#include "RenderStateUtils.h"
#include "RenderWindow.h"
#include "ResolutionServer.h"
#include "Shader.h"
#include "ShaderConstantBuffers.h"
#include "ShaderConstantDatabase.h"
#include "ShaderResourceBuffer.h"
#include "ShaderResourceDatabase.h"
#include "Surface.h"
#include "TextureFromFile.h"
#include "TextureFromMemory.h"
#include "TransientRenderBatch.h"
#include "VertexTypes.h"
#endif
//=============================================================================

#endif

#endif


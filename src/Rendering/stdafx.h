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
#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <d3d11shader.h>
#include <d3dcommon.h>
#include <D3Dcompiler.h>
#include <dxgi.h>
#endif

//=============================================================================
#if SG_ENABLE_PROJECT_INCLUDES_IN_STDAFX
#include "Color.h"
#include "Material.h"
#include "RenderBatch.h"
#include "RenderBatchDico.h"
#include "RenderBatchSet.h"
#include "RenderDevice.h"
#include "RenderState.h"
#include "RenderStateDico.h"
#include "RenderStateUtils.h"
#include "RenderTarget.h"
#include "RenderWindow.h"
#include "ResolutionServer.h"
#include "Shader.h"
#include "ShaderConstantBuffers.h"
#include "ShaderConstantDatabase.h"
#include "ShaderResource.h"
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


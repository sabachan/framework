#ifndef Rendering_ShaderCache_H
#define Rendering_ShaderCache_H

#include "Shader.h"

#include <Core/IntTypes.h>
#include <Core/ArrayView.h>
struct D3D11_INPUT_ELEMENT_DESC;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11ShaderReflection;
struct ID3D11VertexShader;

#define SHADER_CACHE_ENABLE_RELOADABLE_SHADERS SG_ENABLE_TOOLS

namespace sg {
namespace rendering {
class RenderDevice;
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
u64 ComputeShaderReflectionHash(ID3D11ShaderReflection* iShaderReflection);
#endif
namespace shadercache {
//=============================================================================
void Init();
void SetRenderDevice(RenderDevice* iRenderDevice);
// The D3D11_INPUT_ELEMENT_DESC array taken as parameter is assumed to have a lifetime
// greater than the shader cache. Use a global array for instance (cf. VertexTypes).
ShaderInputLayoutProxy GetProxy(ArrayView<D3D11_INPUT_ELEMENT_DESC const> iDescriptor, VertexShaderProxy iVertexShaderForValidation);
VertexShaderProxy GetProxy(VertexShaderDescriptor const* iDescriptor);
PixelShaderProxy GetProxy(PixelShaderDescriptor const* iDescriptor);
ArrayView<D3D11_INPUT_ELEMENT_DESC const> GetInputLayoutDescriptor(ShaderInputLayoutProxy iProxy);
ID3D11InputLayout* GetInputLayout(ShaderInputLayoutProxy iProxy);
ID3D11VertexShader* GetShader(VertexShaderProxy iProxy);
ID3D11PixelShader* GetShader(PixelShaderProxy iProxy);
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
u64 GetShaderTimestamp(VertexShaderProxy iProxy);
u64 GetShaderTimestamp(PixelShaderProxy iProxy);
#endif
ID3D11ShaderReflection* GetReflection(VertexShaderProxy iProxy);
ID3D11ShaderReflection* GetReflection(PixelShaderProxy iProxy);
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
void InvalidateOutdatedShaders();
#endif
void InvalidateAllShaders();
void Shutdown();
//=============================================================================
}
}
}

#endif

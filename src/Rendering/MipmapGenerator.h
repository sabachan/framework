#ifndef Rendering_MipmapGenerator_H
#define Rendering_MipmapGenerator_H

#include <Core/ComPtr.h>
#include "Shader.h"

struct ID3D11BlendState;
struct ID3D11Buffer;
struct ID3D11SamplerState;

namespace sg {
namespace rendering {
//=============================================================================
class RenderDevice;
//class ShaderConstantBuffers;
//class ShaderConstantDatabase;
class Surface;
//=============================================================================
class SimpleMipmapGenerator
{
public:
    SimpleMipmapGenerator(RenderDevice const* iRenderDevice);
    ~SimpleMipmapGenerator();
    void GenerateMipmap(RenderDevice const* iRenderDevice, Surface* iSurface, PixelShaderProxy iShader) const;
private:
    ShaderInputLayoutProxy m_inputLayout;
    VertexShaderProxy m_vertexShader;
    comptr<ID3D11Buffer> m_vertexBuffer;
    comptr<ID3D11BlendState> m_blendState;
    comptr<ID3D11SamplerState> m_samplerState;
    //safeptr<ShaderConstantDatabase const> m_database;
    //scopedptr<ShaderConstantBuffers> m_psConstantBuffers;
    //scopedptr<ShaderConstantBuffers> m_vsConstantBuffers;
};
//=============================================================================
}
}

#endif

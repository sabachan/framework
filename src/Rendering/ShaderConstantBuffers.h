#ifndef Rendering_ShaderConstantBuffer_H
#define Rendering_ShaderConstantBuffer_H

#include <Core/ComPtr.h>
#include <Core/IntTypes.h>
#include <vector>

struct ID3D11Buffer;
struct ID3D11ShaderReflection;

namespace sg {
namespace rendering {
//=============================================================================
class RenderDevice;
class IShaderConstantDatabase;
//=============================================================================
class ShaderConstantBuffers : public SafeCountable
{
public:
    ShaderConstantBuffers();
    ~ShaderConstantBuffers();
    void UpdateIFN(RenderDevice const* iRenderDevice, ID3D11ShaderReflection* iShaderReflection, IShaderConstantDatabase const* iDatabase);
    ID3D11Buffer** Buffers() { return m_buffersPointers.data(); }
    size_t BufferCount() const { return m_buffersPointers.size(); }
private:
    void CreateBuffers(RenderDevice const* iRenderDevice, ID3D11ShaderReflection* iShaderReflection);
private:
    std::vector<ID3D11Buffer*> m_buffersPointers;
    std::vector<comptr<ID3D11Buffer> > m_constantBuffers;
#if SG_ENABLE_TOOLS
    u64 m_shaderReflectionHash;
#endif
};
//=============================================================================
}
}

#endif

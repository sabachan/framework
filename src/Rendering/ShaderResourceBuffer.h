#ifndef Rendering_ShaderResourceBuffer_H
#define Rendering_ShaderResourceBuffer_H

#include <Core/ComPtr.h>
#include <Core/IntTypes.h>
#include <vector>

struct ID3D11SamplerState;
struct ID3D11ShaderReflection;
struct ID3D11ShaderResourceView;

namespace sg {
namespace rendering {
//=============================================================================
class IShaderResource;
class IShaderResourceDatabase;
//=============================================================================
class ShaderResourceBuffer : public SafeCountable
{
public:
    ShaderResourceBuffer();
    ~ShaderResourceBuffer();
    void GetShaderResourceViews(size_t& ioSrvCount, ID3D11ShaderResourceView** oSrvs) const;
    void GetSamplers(size_t& ioSsCount, ID3D11SamplerState** oSss) const;
    void UpdateIFN(ID3D11ShaderReflection* iShaderReflection, IShaderResourceDatabase const* iDatabase);
private:
    std::vector<safeptr<IShaderResource const> > m_shaderResources;
    std::vector<comptr<ID3D11SamplerState> > m_samplers;
#if SG_ENABLE_TOOLS
    u64 m_shaderReflectionHash;
#endif
};
//=============================================================================
}
}

#endif

#ifndef Rendering_Material_H
#define Rendering_Material_H

#include "RenderStateDico.h"
#include "Shader.h"
#include "ShaderConstantDatabase.h"
#include "ShaderResourceDatabase.h"

// A Material represents a set of rendering parameters. A material will be used
// along with other resource and constant databases to completely set the
// rendering parameters. The constants and resources that are set in the
// material will be used preferably to those from other databases.
//
// - Why taking IShaderVariables by pointers and not copying them?
//      This way, client code can modify shader variables in order to animate
//      the material instead of having to generate a new material for each
//      possible value and making dico growing huge.
//      The drawback is that 2 materials created with same shaders and same
//      constant values won't be merged if the IShaderVariable is not the same.
//      I assume that this case is sufficently rare.

namespace sg {
namespace rendering {
//=============================================================================
class Material : public RefAndSafeCountable
{
    Material & operator=(Material const&) = delete;
    friend bool operator==(Material const& m1, Material const& m2);
public:
    typedef ArrayView<std::pair<ShaderConstantName, IShaderVariable*> const> ConstantsView;
    typedef ArrayView<std::pair<ShaderResourceName, IShaderResource const*> const> ResourcesView;
    typedef ArrayView<std::pair<ShaderResourceName, ID3D11SamplerState*> const> SamplersView;
    Material(ShaderInputLayoutProxy iInputLayout,
             VertexShaderProxy iVS,
             PixelShaderProxy iPS,
             RenderStateName iBlendMode,
             ConstantsView iConstants = nullptr,
             ResourcesView iResources = nullptr,
             SamplersView iSamplers = nullptr,
             int iPriority = 0);
    Material(Material const& iOther);
    Material(Material const& iOther,
             ConstantsView iConstants,
             ResourcesView iResources = nullptr,
             SamplersView iSamplers = nullptr);
    size_t Hash() const;
    ShaderInputLayoutProxy InputLayout() const { return m_inputLayout; }
    VertexShaderProxy VertexShader() const { return m_vertexShader; }
    PixelShaderProxy PixelShader() const { return m_pixelShader; }
    RenderStateName BlendMode() const { return m_blendMode; }
    ShaderConstantDatabase const& Constants() const { return m_constants; }
    ShaderResourceDatabase const& Resources() const { return m_resources; }
    int Priority() const { return m_priority; }
private:
    size_t ComputeHash() const;
private:
    size_t m_hash;
    int m_priority;
    ShaderInputLayoutProxy m_inputLayout;
    VertexShaderProxy m_vertexShader;
    PixelShaderProxy m_pixelShader;
    RenderStateName m_blendMode;
    ShaderConstantDatabase m_constants;
    ShaderResourceDatabase m_resources;
};
//=============================================================================
bool operator==(Material const& m1, Material const& m2);
//=============================================================================
}
}

#endif

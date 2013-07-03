#include "stdafx.h"

#include "Material.h"

#include "ShaderResource.h"
#include <d3d11.h>

namespace sg {
namespace rendering {
//=============================================================================
Material::Material(ShaderInputLayoutProxy iInputLayout,
                   VertexShaderProxy iVS,
                   PixelShaderProxy iPS,
                   RenderStateName iBlendMode,
                   ConstantsView iConstants,
                   ResourcesView iResources,
                   SamplersView iSamplers,
                   int iPriority)
: m_priority(iPriority)
, m_inputLayout(iInputLayout)
, m_vertexShader(iVS)
, m_pixelShader(iPS)
, m_blendMode(iBlendMode)
, m_constants()
, m_resources()
{
    for(auto const& it : iConstants)
        m_constants.AddVariable(it.first, it.second);
    for(auto const& it : iResources)
        m_resources.AddResource(it.first, it.second);
    for(auto const& it : iSamplers)
        m_resources.AddSampler(it.first, it.second);
    m_hash = ComputeHash();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Material::Material(Material const& iOther)
: m_hash(iOther.m_hash)
, m_priority(iOther.m_priority)
, m_inputLayout(iOther.m_inputLayout)
, m_vertexShader(iOther.m_vertexShader)
, m_pixelShader(iOther.m_pixelShader)
, m_blendMode(iOther.m_blendMode)
, m_constants(iOther.m_constants) // Now, such copy is correct, but will database class evole ?
, m_resources(iOther.m_resources) // Now, such copy is correct, but will database class evole ?
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Material::Material(Material const& iOther,
                   ConstantsView iConstants,
                   ResourcesView iResources,
                   SamplersView iSamplers)
: m_priority(iOther.m_priority)
, m_inputLayout(iOther.m_inputLayout)
, m_vertexShader(iOther.m_vertexShader)
, m_pixelShader(iOther.m_pixelShader)
, m_blendMode(iOther.m_blendMode)
, m_constants(iOther.m_constants) // Now, such copy is correct, but will database class evole ?
, m_resources(iOther.m_resources) // Now, such copy is correct, but will database class evole ?
{
    for(auto const& it : iConstants)
        m_constants.AddVariable(it.first, it.second);
    for(auto const& it : iResources)
        m_resources.AddResource(it.first, it.second);
    for(auto const& it : iSamplers)
        m_resources.AddSampler(it.first, it.second);
    m_hash = ComputeHash();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Material::Hash() const
{
    SG_ASSERT(ComputeHash() == m_hash);
    return m_hash;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t Material::ComputeHash() const
{
    // Note that constants values are not used to compute hash.
    // We prefer to consider that a shader constant can be modified by client
    // from frame to frame. It implies that Materials having same constants values
    // cannot be merged, but we assume that this case is rare.
    size_t const rot = 7;
#define BIT_ROTATE(A, N) do { A = (A << N) ^ (A >> (8*sizeof(h)-N)); } while(SG_CONSTANT_CONDITION(0))
    size_t h = m_priority;
    BIT_ROTATE(h, rot);
    h ^= m_inputLayout.Hash();
    BIT_ROTATE(h, rot);
    h ^= m_vertexShader.Hash();
    BIT_ROTATE(h, rot);
    h ^= m_pixelShader.Hash();
    BIT_ROTATE(h, rot);
    h ^= m_blendMode.Hash();
    BIT_ROTATE(h, rot);
    h ^= m_constants.Constants().size();
    BIT_ROTATE(h, rot);
    h ^= m_resources.Resources().size();
    BIT_ROTATE(h, rot);
    h ^= m_resources.Samplers().size();
    for(auto const& it : m_constants.Constants())
    {
        BIT_ROTATE(h, rot);
        h ^= it.first.Hash();
        BIT_ROTATE(h, rot);
        h ^= ptrdiff_t(it.second.get());
    }
    for(auto const& it : m_resources.Resources())
    {
        BIT_ROTATE(h, rot);
        h ^= it.first.Hash();
        BIT_ROTATE(h, rot);
        h ^= ptrdiff_t(it.second.get());
    }
    for(auto const& it : m_resources.Samplers())
    {
        BIT_ROTATE(h, rot);
        h ^= it.first.Hash();
        BIT_ROTATE(h, rot);
        h ^= ptrdiff_t(it.second.get());
    }
    BIT_ROTATE(h, rot);
#undef BIT_ROTATE
    h ^= m_blendMode.Hash();
    return h;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool operator==(Material const& m1, Material const& m2)
{
    if(m1.Hash() != m2.Hash())
        return false;
    if(m1.m_priority != m2.m_priority)
        return false;
    if(m1.m_inputLayout != m2.m_inputLayout)
        return false;
    if(m1.m_vertexShader != m2.m_vertexShader)
        return false;
    if(m1.m_pixelShader != m2.m_pixelShader)
        return false;
    if(m1.m_blendMode != m2.m_blendMode)
        return false;

    auto const& constants1 = m1.m_constants.Constants();
    auto const& resources1 = m1.m_resources.Resources();
    auto const& samplers1 = m1.m_resources.Samplers();
    auto const& constants2 = m2.m_constants.Constants();
    auto const& resources2 = m2.m_resources.Resources();
    auto const& samplers2 = m2.m_resources.Samplers();

    if(constants1.size() != constants2.size())
        return false;
    if(resources1.size() != resources2.size())
        return false;
    if(samplers1.size() != samplers2.size())
        return false;

    {
        auto begin1 = constants1.begin();
        auto begin2 = constants2.begin();
        auto end1 = constants1.end();
        auto end2 = constants2.end();
        for(auto it1 = begin1, it2 = begin2; it1 != end1; ++it1, ++it2)
        {
            SG_ASSERT(it2 != end2);
            if(it1->first != it2->first)
                return false;
            if(it1->second != it2->second)
                return false;
        }
    }
    {
        auto begin1 = resources1.begin();
        auto begin2 = resources2.begin();
        auto end1 = resources1.end();
        auto end2 = resources2.end();
        for(auto it1 = begin1, it2 = begin2; it1 != end1; ++it1, ++it2)
        {
            SG_ASSERT(it2 != end2);
            if(it1->first != it2->first)
                return false;
            if(it1->second != it2->second)
                return false;
        }
    }
    {
        auto begin1 = samplers1.begin();
        auto begin2 = samplers2.begin();
        auto end1 = samplers1.end();
        auto end2 = samplers2.end();
        for(auto it1 = begin1, it2 = begin2; it1 != end1; ++it1, ++it2)
        {
            SG_ASSERT(it2 != end2);
            if(it1->first != it2->first)
                return false;
            if(it1->second != it2->second)
                return false;
        }
    }

    if(m1.m_blendMode != m2.m_blendMode)
        return false;

    return true;
}
//=============================================================================
}
}

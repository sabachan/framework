#include "stdafx.h"

#include "Shader.h"

#include "ShaderCache.h"
#include <Core/FilePath.h>

namespace sg {
namespace rendering {
//=============================================================================
ID3D11VertexShader* VertexShaderProxy::GetShader() const
{
    SG_ASSERT(IsValid());
    return shadercache::GetShader(*this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11ShaderReflection* VertexShaderProxy::GetReflection() const
{
    SG_ASSERT(IsValid());
    return shadercache::GetReflection(*this);
}
//=============================================================================
ID3D11PixelShader* PixelShaderProxy::GetShader() const
{
    SG_ASSERT(IsValid());
    return shadercache::GetShader(*this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11ShaderReflection* PixelShaderProxy::GetReflection() const
{
    SG_ASSERT(IsValid());
    return shadercache::GetReflection(*this);
}
//=============================================================================
ShaderInputLayoutProxy::ShaderInputLayoutProxy(ArrayView<D3D11_INPUT_ELEMENT_DESC const> iDescriptor, VertexShaderProxy iVertexShaderForValidation)
{
    *this = shadercache::GetProxy(iDescriptor, iVertexShaderForValidation);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ArrayView<D3D11_INPUT_ELEMENT_DESC const> ShaderInputLayoutProxy::GetDescriptor() const
{
    SG_ASSERT(IsValid());
    return shadercache::GetInputLayoutDescriptor(*this);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11InputLayout* ShaderInputLayoutProxy::GetInputLayout() const
{
    SG_ASSERT(IsValid());
    return shadercache::GetInputLayout(*this);
}
//=============================================================================
ShaderBaseDescriptor::ShaderBaseDescriptor(FilePath const& iFilePath, std::string const& iEntryPoint, ArrayView<std::pair<std::string, std::string> const> iDefines)
    : m_filepath(iFilePath)
    , m_entryPoint(iEntryPoint)
{
    m_defines.reserve(iDefines.Size());
    for(auto const& it : iDefines)
        m_defines.EmplaceBack_AssumeCapacity(it.first, it.second);
    auto b = m_defines.begin();
    auto e = m_defines.end();
    std::sort(b, e, [](std::pair<std::string, std::string> const& a, std::pair<std::string, std::string>& b) { return a.first < b.first; });
#if SG_ENABLE_ASSERT
    auto u = std::unique(b, e, [](std::pair<std::string, std::string> const& a, std::pair<std::string, std::string>& b) { return a.first == b.first; });
    SG_ASSERT(u == e);
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
size_t ShaderBaseDescriptor::Hash() const
{
    std::hash<FilePath> hfp;
    std::hash<std::string> hs;
    std::hash<size_t> hi;

    size_t h = hfp(m_filepath);
    h ^= hs(m_entryPoint);
    h ^= hi(m_defines.Size());
    for(auto const& it : m_defines)
    {
        h *= 13;
        h ^= hs(it.first);
        h ^= hs(it.second);
    }
    return h;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ShaderBaseDescriptor::Equals(ShaderBaseDescriptor const& a) const
{
    if(m_filepath != a.m_filepath)
        return false;
    if(m_entryPoint != a.m_entryPoint)
        return false;
    if(m_defines.Size() != a.m_defines.Size())
        return false;
    for_range(size_t, i, 0, m_defines.Size())
    {
        if(m_defines[i].first != a.m_defines[i].first)
            return false;
        if(m_defines[i].second != a.m_defines[i].second)
            return false;
    }
    return true;
}
//=============================================================================
size_t VertexShaderDescriptor::Hash() const
{
    return ShaderBaseDescriptor::Hash();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool VertexShaderDescriptor::Equals(VertexShaderDescriptor const& a) const
{
    return ShaderBaseDescriptor::Equals(a);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
VertexShaderProxy VertexShaderDescriptor::GetProxy() const
{
    return shadercache::GetProxy(this);
}
//=============================================================================
size_t PixelShaderDescriptor::Hash() const
{
    return ShaderBaseDescriptor::Hash();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool PixelShaderDescriptor::Equals(PixelShaderDescriptor const& a) const
{
    return ShaderBaseDescriptor::Equals(a);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PixelShaderProxy PixelShaderDescriptor::GetProxy() const
{
    return shadercache::GetProxy(this);
}
//=============================================================================
}
}

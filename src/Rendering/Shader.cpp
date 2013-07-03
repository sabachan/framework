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
size_t ShaderBaseDescriptor::Hash() const
{
    std::hash<FilePath> hfp;
    std::hash<std::string> hs;

    return hfp(m_filepath) ^ hs(m_entryPoint);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ShaderBaseDescriptor::Equals(ShaderBaseDescriptor const& a) const
{
    return     (m_filepath == a.m_filepath)
            && (m_entryPoint == a.m_entryPoint);
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

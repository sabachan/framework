#include "stdafx.h"

#include "ShaderResourceBuffer.h"

#include "IShaderResource.h"
#include "ShaderCache.h"
#include "ShaderResourceDatabase.h"
#include <Core/Log.h>
#include <sstream>

#include "WTF/IncludeD3D11.h"
#include "WTF/IncludeD3D11Shader.h"

namespace sg {
namespace rendering {
//=============================================================================
ShaderResourceBuffer::ShaderResourceBuffer()
    : m_shaderResources()
#if SG_ENABLE_TOOLS
    , m_shaderReflectionHash(0)
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderResourceBuffer::~ShaderResourceBuffer()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderResourceBuffer::GetShaderResourceViews(size_t& ioSrvCount, ID3D11ShaderResourceView** oSrvs) const
{
    size_t const count = m_shaderResources.size();
    SG_ASSERT(count <= ioSrvCount);
    ioSrvCount = count;
    for(size_t i = 0; i < count; ++i)
        oSrvs[i] = m_shaderResources[i]->GetShaderResourceView();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderResourceBuffer::GetSamplers(size_t& ioSsCount, ID3D11SamplerState** oSss) const
{
    size_t const count = m_samplers.size();
    SG_ASSERT(count <= ioSsCount);
    ioSsCount = count;
    for(size_t i = 0; i < count; ++i)
        oSss[i] = m_samplers[i].get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderResourceBuffer::UpdateIFN(ID3D11ShaderReflection* iShaderReflection, IShaderResourceDatabase const* iDatabase)
{
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    u64 const shaderReflectionHash = ComputeShaderReflectionHash(iShaderReflection);
    if(shaderReflectionHash != m_shaderReflectionHash)
    {
        m_shaderResources.clear();
        m_samplers.clear();
        m_shaderReflectionHash = shaderReflectionHash;
    }
#endif
    if(m_shaderResources.empty())
    {
        D3D11_SHADER_DESC shDesc;
        HRESULT hr = iShaderReflection->GetDesc(&shDesc);
        SG_ASSERT(SUCCEEDED(hr));
        for(UINT i = 0; i < shDesc.BoundResources; ++i)
        {
            D3D11_SHADER_INPUT_BIND_DESC bindDesc;
            hr = iShaderReflection->GetResourceBindingDesc(i, &bindDesc);
            LPCSTR name = bindDesc.Name;
            ShaderResourceName resourceName(name);

            switch(bindDesc.Type)
            {
            case D3D_SIT_CBUFFER:
                //SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_TBUFFER:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_TEXTURE:
                SG_ASSERT(1 == bindDesc.BindCount);
                SG_ASSERT(m_shaderResources.size() == bindDesc.BindPoint);
                SG_ASSERT(D3D_SRV_DIMENSION_TEXTURE2D == bindDesc.Dimension);
                SG_ASSERT(-1 == bindDesc.NumSamples);
#if 0
                switch(bindDesc.Type)
                {
                case D3D_RETURN_TYPE_UNORM:
                    SG_ASSERT_NOT_IMPLEMENTED();
                    break;
                case D3D_RETURN_TYPE_SNORM:
                    SG_ASSERT_NOT_IMPLEMENTED();
                    break;
                case D3D_RETURN_TYPE_SINT:
                    SG_ASSERT_NOT_IMPLEMENTED();
                    break;
                case D3D_RETURN_TYPE_UINT:
                    SG_ASSERT_NOT_IMPLEMENTED();
                    break;
                case D3D_RETURN_TYPE_FLOAT:
                    SG_ASSERT_NOT_IMPLEMENTED();
                    break;
                case D3D_RETURN_TYPE_MIXED:
                    SG_ASSERT_NOT_IMPLEMENTED();
                    break;
                case D3D_RETURN_TYPE_DOUBLE:
                    SG_ASSERT_NOT_IMPLEMENTED();
                    break;
                case D3D_RETURN_TYPE_CONTINUED:
                    SG_ASSERT_NOT_IMPLEMENTED();
                    break;
                default:
                    SG_ASSERT_NOT_REACHED();
                }
#endif
                {
                    SG_ASSERT(1 == bindDesc.BindCount);
                    SG_ASSERT(m_shaderResources.size() == bindDesc.BindPoint);
                    IShaderResource const* resource = iDatabase->GetResourceIFP(resourceName);
                    SG_ASSERT(nullptr != resource);
                    if(nullptr == resource)
                    {
                         // TODO: Error message
                    }
                    m_shaderResources.emplace_back(resource);
                }
                break;
            case D3D_SIT_SAMPLER:
                {
                    SG_ASSERT(1 == bindDesc.BindCount);
                    SG_ASSERT(m_samplers.size() == bindDesc.BindPoint);
                    ID3D11SamplerState* sampler = iDatabase->GetSamplerIFP(resourceName);
                    SG_ASSERT(nullptr != sampler);
                    if(nullptr == sampler)
                    {
                         // TODO: Error message
                    }
                    m_samplers.emplace_back(sampler);
                }
                break;
            case D3D_SIT_UAV_RWTYPED:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_STRUCTURED:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_UAV_RWSTRUCTURED:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_BYTEADDRESS:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_UAV_RWBYTEADDRESS:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_UAV_APPEND_STRUCTURED:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                SG_ASSERT_NOT_IMPLEMENTED();
                break;
            default:
                SG_ASSERT_NOT_REACHED();
            }
        }
    }
}
//=============================================================================
}
}

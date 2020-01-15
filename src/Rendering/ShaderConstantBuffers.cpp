#include "stdafx.h"

#include "ShaderConstantBuffers.h"

#include <Core/Log.h>
#include "RenderDevice.h"
#include "ShaderCache.h"
#include "ShaderConstantDatabase.h"
#include <sstream>

#include "WTF/IncludeD3D11.h"
#include "WTF/IncludeD3D11Shader.h"

namespace sg {
namespace rendering {
//=============================================================================
ShaderConstantBuffers::ShaderConstantBuffers()
    : m_buffersPointers()
    , m_constantBuffers()
#if SG_ENABLE_TOOLS
    , m_shaderReflectionHash(0)
#endif
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderConstantBuffers::~ShaderConstantBuffers()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderConstantBuffers::UpdateIFN(RenderDevice const* iRenderDevice, ID3D11ShaderReflection* iShaderReflection, IShaderConstantDatabase const* iDatabase)
{
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    u64 const shaderReflectionHash = ComputeShaderReflectionHash(iShaderReflection);
    if(shaderReflectionHash != m_shaderReflectionHash)
    {
        m_buffersPointers.clear();
        m_constantBuffers.clear();
        m_shaderReflectionHash = shaderReflectionHash;
    }
#endif
    if(m_constantBuffers.empty())
        CreateBuffers(iRenderDevice, iShaderReflection);

    ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    D3D11_SHADER_DESC shDesc;
    HRESULT hr = iShaderReflection->GetDesc(&shDesc);
    SG_ASSERT(SUCCEEDED(hr));
    SG_ASSERT(m_constantBuffers.size() == shDesc.ConstantBuffers);
    for(UINT i = 0; i < shDesc.ConstantBuffers; ++i)
    {
        ID3D11ShaderReflectionConstantBuffer* cb = iShaderReflection->GetConstantBufferByIndex(i);
        SG_ASSERT(nullptr != cb);
        D3D11_SHADER_BUFFER_DESC cbDesc;
        hr = cb->GetDesc(&cbDesc);
        SG_ASSERT(SUCCEEDED(hr));
#if SG_ENABLE_ASSERT
        D3D11_BUFFER_DESC bufferDesc;
        m_constantBuffers[i]->GetDesc(&bufferDesc);
        SG_ASSERT(cbDesc.Size == bufferDesc.ByteWidth);
#endif
        D3D11_MAPPED_SUBRESOURCE mapped;
        hr = context->Map(
            m_constantBuffers[i].get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mapped);
        SG_ASSERT(SUCCEEDED(hr));
        for(UINT v = 0; v < cbDesc.Variables; ++v)
        {
            ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(v);
            D3D11_SHADER_VARIABLE_DESC varDesc;
            hr = var->GetDesc(&varDesc);
            SG_ASSERT(SUCCEEDED(hr));
            if(0 != (varDesc.uFlags & D3D_SVF_USED))
            {
                ID3D11ShaderReflectionType* varType = var->GetType();
                SG_ASSERT(nullptr != varType);
                D3D11_SHADER_TYPE_DESC varTypeDesc;
                hr = varType->GetDesc(&varTypeDesc);
                SG_ASSERT(SUCCEEDED(hr));
                SG_ASSERT(0 == varTypeDesc.Members); // not impl.
                size_t const startOffset = varDesc.StartOffset;
                size_t const size = varDesc.Size;
                // TODO: try to keep the fast symbols instead of reconstructing them
                ShaderConstantName constantname(varDesc.Name);
                IShaderVariable const* constant = iDatabase->GetConstant(constantname);
                SG_ASSERT(nullptr != constant);
                if(nullptr != constant)
                {
                    // TODO: Check type
                    //if(...)
                    //{
                    //    std::ostringstream oss;
                    //    oss << "Shader variable " << varDesc.Name << " has not the correct type." << std::endl;
                    //    SG_LOG_WARNING("Rendering/Shader", oss.str().c_str());
                    //}

                    constant->WriteInBuffer((u8*)mapped.pData + startOffset, size);
                }
                else
                {
                    // TODO: Assert initialized to 0 ?
                }
                SG_BREAKABLE_POS;
            }
        }
        context->Unmap(
            m_constantBuffers[i].get(),
            0);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderConstantBuffers::CreateBuffers(RenderDevice const* iRenderDevice, ID3D11ShaderReflection* iShaderReflection)
{
    ID3D11Device* device = iRenderDevice->D3DDevice();
    SG_ASSERT(nullptr != device);

    D3D11_SHADER_DESC shDesc;
    HRESULT hr = iShaderReflection->GetDesc(&shDesc);
    SG_ASSERT(SUCCEEDED(hr));
    for(UINT i = 0; i < shDesc.ConstantBuffers; ++i)
    {
        ID3D11ShaderReflectionConstantBuffer* cb = iShaderReflection->GetConstantBufferByIndex(i);
        SG_ASSERT(nullptr != cb);
        D3D11_SHADER_BUFFER_DESC cbDesc;
        hr = cb->GetDesc(&cbDesc);
        SG_ASSERT(SUCCEEDED(hr));
        // TODO: try to merge constant buffers of different shaders ?
        comptr<ID3D11Buffer> constantBuffer;
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth           = cbDesc.Size;
        bufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags           = 0;
        bufferDesc.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = nullptr;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetPointerForInitialisation());
        SG_ASSERT(SUCCEEDED(hr));
        m_constantBuffers.push_back(constantBuffer);
        m_buffersPointers.push_back(constantBuffer.get());

        for(UINT v = 0; v < cbDesc.Variables; ++v)
        {
            ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(v);
            D3D11_SHADER_VARIABLE_DESC varDesc;
            hr = var->GetDesc(&varDesc);
            SG_ASSERT(SUCCEEDED(hr));
            ID3D11ShaderReflectionType* varType = var->GetType();
            SG_ASSERT(nullptr != varType);
            D3D11_SHADER_TYPE_DESC varTypeDesc;
            hr = varType->GetDesc(&varTypeDesc);
            SG_ASSERT(SUCCEEDED(hr));
            SG_ASSERT(0 == varTypeDesc.Members); // not impl.

            if(0 == (varDesc.uFlags & D3D_SVF_USED))
            {
                std::ostringstream oss;
                oss << "Shader variable " << varDesc.Name << " is not used." << std::endl;
                SG_LOG_WARNING("Rendering/Shader", oss.str().c_str());
            }
        }
    }
}
//=============================================================================
}
}

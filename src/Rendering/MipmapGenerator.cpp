#include "stdafx.h"

#include "MipmapGenerator.h"

#include <Core/Cast.h>
#include "RenderDevice.h"
#include "RenderStateUtils.h"
#include "Surface.h"
#include "VertexTypes.h"
#include <algorithm>
#include <d3d11.h>
#include <d3d11shader.h>

namespace sg {
namespace rendering {
//=============================================================================
namespace {
    typedef Vertex_Pos2f_Tex2f MipmapVertex;
}
//=============================================================================
SimpleMipmapGenerator::SimpleMipmapGenerator(RenderDevice const* iRenderDevice)
{
    ID3D11Device* device = iRenderDevice->D3DDevice();
    SG_ASSERT(nullptr != device);

    refptr<VertexShaderDescriptor> vsDesc = new VertexShaderDescriptor(FilePath("src:/Rendering/Shaders/Mipmap_Vertex.hlsl"), "vmain");
    m_vertexShader = vsDesc->GetProxy();
    m_inputLayout = ShaderInputLayoutProxy(MipmapVertex::InputEltDesc(), m_vertexShader);

    MipmapVertex vertices[] =
    {
        { float2(-1,  1), float2(0, 0) },
        { float2(-1, -1), float2(0, 1) },
        { float2( 1,  1), float2(1, 0) },
        { float2( 1, -1), float2(1, 1) },
    };
    size_t vertexCount = SG_ARRAYSIZE(vertices);

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage               = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth           = num_cast<UINT>( sizeof( MipmapVertex ) * vertexCount );
    bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags      = 0;
    bufferDesc.MiscFlags           = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_vertexBuffer.GetPointerForInitialisation() );
    SG_ASSERT(SUCCEEDED(hr));

    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = false;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = device->CreateBlendState(&blendDesc, m_blendState.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 0;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0.f;
    samplerDesc.BorderColor[1] = 0.f;
    samplerDesc.BorderColor[2] = 0.f;
    samplerDesc.BorderColor[3] = 0.f;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = 0;

    hr = device->CreateSamplerState(&samplerDesc, m_samplerState.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SimpleMipmapGenerator::~SimpleMipmapGenerator()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SimpleMipmapGenerator::GenerateMipmap(RenderDevice const* iRenderDevice, Surface* iSurface, PixelShaderProxy iShader) const
{
    ID3D11DeviceContext* context = iRenderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    SaveRenderStateRestoreAtEnd saveRenderStateRestoreAtEnd(iRenderDevice);

    //{
    //    ID3D11ShaderReflection* reflection = m_pixelShader.GetReflection();
    //    m_psConstantBuffers->UpdateIFN(iRenderDevice, reflection, m_database.get());
    //    ID3D11Buffer** buffers = m_psConstantBuffers->Buffers();
    //    UINT bufferCount = m_psConstantBuffers->BufferCount();
    //    context->PSSetConstantBuffers(
    //        0,
    //        bufferCount,
    //        buffers);
    //}

    //{
    //    ID3D11ShaderReflection* reflection = m_vertexShader.GetReflection();
    //    m_vsConstantBuffers->UpdateIFN(iRenderDevice, reflection, m_database.get());
    //    ID3D11Buffer** buffers = m_vsConstantBuffers->Buffers();
    //    UINT bufferCount = m_vsConstantBuffers->BufferCount();
    //    context->VSSetConstantBuffers(
    //        0,
    //        bufferCount,
    //        buffers);
    //}

    UINT const stride = sizeof( MipmapVertex );
    UINT const offset = 0;

    ID3D11Buffer* vertexBuffers[] =
    {
        m_vertexBuffer.get()
    };
    UINT const vertexBufferCount = SG_ARRAYSIZE(vertexBuffers);

    context->IASetVertexBuffers(
        0,                // the first input slot for binding
        vertexBufferCount,// the number of buffers in the array
        vertexBuffers,    // the array of vertex buffers
        &stride,          // array of stride values, one for each buffer
        &offset );        // array of offset values, one for each buffer

    // Set the input layout
    context->IASetInputLayout( m_inputLayout.GetInputLayout() );
    context->VSSetShader( m_vertexShader.GetShader(), NULL, 0 );
    context->PSSetShader( iShader.GetShader(), NULL, 0 );

    context->OMSetBlendState( m_blendState.get(), NULL, 0xFFFFFFFF );

    ID3D11SamplerState* sss[] = {
        m_samplerState.get(),
    };
    size_t const ssCount = SG_ARRAYSIZE(sss);
    context->PSSetSamplers( 0, ssCount, sss );

    uint2 wh = iSurface->Resolution()->Get();

    size_t const mipCount = iSurface->MipCount();
    for(size_t i = 1; i < mipCount; ++i)
    {
        uint2 mipwh = uint2(std::max<u32>(wh.x()>>i, 1), std::max<u32>(wh.y()>>i, 1));

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)mipwh.x();
        vp.Height = (FLOAT)mipwh.y();
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        context->RSSetViewports( 1, &vp );

        // Setup a scissor rect
        D3D11_RECT rect;
        rect.left = (int)vp.TopLeftX;
        rect.top = (int)vp.TopLeftY;
        rect.right = (int)(vp.Width - vp.TopLeftX);
        rect.bottom = (int)(vp.Height - vp.TopLeftY);
        context->RSSetScissorRects( 1, &rect );

        ID3D11RenderTargetView* rtvs[] = {
            iSurface->GetMipLevelRenderTargetView(i),
        };
        size_t const rtvCount = SG_ARRAYSIZE(rtvs);

        iRenderDevice->ImmediateContext()->OMSetRenderTargets( rtvCount, rtvs, NULL );

        ID3D11ShaderResourceView* srvs[] = {
           iSurface->GetMipLevelShaderResourceView(i-1),
        };
        size_t const srvCount = SG_ARRAYSIZE(srvs);
        context->PSSetShaderResources( 0, srvCount, srvs );

        context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

        context->Draw(4, 0);

        ID3D11ShaderResourceView* clearsrvs[] = {
            nullptr,
        };
        size_t const clearsrvCount = SG_ARRAYSIZE(clearsrvs);
        context->PSSetShaderResources( 0, clearsrvCount, clearsrvs );
    }
}
//=============================================================================
}
}

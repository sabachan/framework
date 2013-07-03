#include "stdafx.h"

#include "CompositingInstruction.h"

#include <Core/Log.h>
#include <Reflection/CommonTypes.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/RenderStateUtils.h>
#include <Rendering/ShaderConstantBuffers.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <Rendering/ShaderResourceBuffer.h>
#include <Rendering/Surface.h>
#include <Rendering/VertexTypes.h>
#include <algorithm>
#include <d3d11.h>
#include <d3d11shader.h>
#include "Compositing.h"
#include "DatabaseDescriptors.h"
#include "RenderBatch.h"
#include "ShaderDescriptors.h"
#include "SurfaceDescriptors.h"

namespace sg {
namespace renderengine {
//=============================================================================
namespace {
    typedef rendering::Vertex_Pos2f_Tex2f QuadForShaderPassVertex;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
QuadForShaderPass::QuadForShaderPass(rendering::RenderDevice const* iRenderDevice)
    : m_vsConstantBuffers(new rendering::ShaderConstantBuffers())
{
    ID3D11Device* device = iRenderDevice->D3DDevice();
    SG_ASSERT(nullptr != device);

    refptr<rendering::VertexShaderDescriptor> vsDesc = new rendering::VertexShaderDescriptor(FilePath("src:/RenderEngine/Shaders/QuadForShaderPass_Vertex.hlsl"), "vmain");
    m_defaultVertexShader = vsDesc->GetProxy();

    m_inputLayout = rendering::ShaderInputLayoutProxy(QuadForShaderPassVertex::InputEltDesc(), m_defaultVertexShader);

    QuadForShaderPassVertex vertices[] =
    {
        { float2(-1,  1), float2(0, 0) },
        { float2(-1, -1), float2(0, 1) },
        { float2( 1,  1), float2(1, 0) },
        { float2( 1, -1), float2(1, 1) },
    };
    size_t vertexCount = SG_ARRAYSIZE(vertices);

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage               = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth           = num_cast<UINT>( sizeof( QuadForShaderPassVertex ) * vertexCount );
    bufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags      = 0;
    bufferDesc.MiscFlags           = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    HRESULT hr = device->CreateBuffer( &bufferDesc, &InitData, m_vertexBuffer.GetPointerForInitialisation() );
    SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
QuadForShaderPass::~QuadForShaderPass()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void QuadForShaderPass::Execute(Compositing* iCompositing)
{
    rendering::RenderDevice const* renderDevice = iCompositing->RenderDevice();
    ID3D11DeviceContext* context = renderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    UINT const stride = sizeof( QuadForShaderPassVertex );
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

    context->IASetInputLayout( m_inputLayout.GetInputLayout() );

    context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    context->Draw(4, 0);
}
//=============================================================================
AbstractCompositingInstructionDescriptor::AbstractCompositingInstructionDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
AbstractCompositingInstructionDescriptor::~AbstractCompositingInstructionDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_ABSTRACT_CLASS_BEGIN((sg, renderengine), AbstractCompositingInstructionDescriptor)
REFLECTION_CLASS_DOC("")
REFLECTION_CLASS_END
//=============================================================================
ClearSurfaces::ClearSurfaces(ClearSurfacesDescriptor const* iDescriptor, Compositing* iCompositing)
    : m_descriptor(iDescriptor)
{
    m_renderTargets.reserve(m_descriptor->m_renderTargets.size());
    for(auto const& it : m_descriptor->m_renderTargets)
    {
        m_renderTargets.push_back(it->GetAsRenderTarget(iCompositing));
        SG_ASSERT(nullptr != m_renderTargets.back());
    }
    m_depthStencils.reserve(m_descriptor->m_depthStencils.size());
    for(auto const& it : m_descriptor->m_depthStencils)
    {
        m_depthStencils.push_back(it->GetAsSurface(iCompositing));
        SG_ASSERT(nullptr != m_depthStencils.back());
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ClearSurfaces::~ClearSurfaces()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ClearSurfaces::Execute(Compositing* iCompositing)
{
    rendering::RenderDevice const* renderDevice = iCompositing->RenderDevice();
    for(auto const& it : m_renderTargets)
    {
        SG_ASSERT(nullptr != it);
        SG_ASSERT(nullptr != it->GetRenderTargetView());
        float const* color = reinterpret_cast<float const*>(&(m_descriptor->m_color));
        renderDevice->ImmediateContext()->ClearRenderTargetView(it->GetRenderTargetView(), color);
    }
    for(auto const& it : m_depthStencils)
    {
        SG_ASSERT(nullptr != it);
        SG_ASSERT(nullptr != it->GetDepthStencilView());
        float const depth = m_descriptor->m_depth;
        UINT8 const stencil = m_descriptor->m_stencil;
        renderDevice->ImmediateContext()->ClearDepthStencilView(it->GetDepthStencilView(), D3D11_CLEAR_DEPTH, depth, stencil);
    }
}
//=============================================================================
ClearSurfacesDescriptor::ClearSurfacesDescriptor()
: m_renderTargets()
, m_color()
, m_depthStencils()
, m_depth(0)
, m_stencil(0)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ClearSurfacesDescriptor::~ClearSurfacesDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ClearSurfaces* ClearSurfacesDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new ClearSurfaces(this, iCompositing);
}
// TODO: Check properties
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), ClearSurfacesDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(renderTargets, "")
    REFLECTION_m_PROPERTY_DOC(color, "clear color")
    REFLECTION_m_PROPERTY_DOC(depthStencils, "")
    REFLECTION_m_PROPERTY_DOC(depth, "")
    REFLECTION_m_PROPERTY_DOC(stencil, "")
REFLECTION_CLASS_END
//=============================================================================
GenerateMipmap::GenerateMipmap(GenerateMipmapDescriptor const* iDescriptor, Compositing* iCompositing)
    : m_descriptor(iDescriptor)
    , m_psConstantBuffers(new rendering::ShaderConstantBuffers())
    , m_vsConstantBuffers(new rendering::ShaderConstantBuffers())
    , m_psResources(new rendering::ShaderResourceBuffer())
    , m_vsResources(new rendering::ShaderResourceBuffer())
{
    SG_ASSERT(nullptr != m_descriptor->m_pixelShader);
    m_pixelShader = m_descriptor->m_pixelShader->GetProxy();
    if(nullptr != m_descriptor->m_vertexShader)
        m_vertexShader = m_descriptor->m_vertexShader->GetProxy();
    else
        m_vertexShader = iCompositing->GetQuadForShaderPass()->DefaultVertexShader();
    SG_ASSERT(nullptr != m_descriptor->m_surface);
    m_surface = m_descriptor->m_surface->GetAsSurface(iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
GenerateMipmap::~GenerateMipmap()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GenerateMipmap::Execute(Compositing* iCompositing)
{
    rendering::RenderDevice const* renderDevice = iCompositing->RenderDevice();
    ID3D11DeviceContext* context = renderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    rendering::SaveRenderStateRestoreAtEnd saveRenderStateRestoreAtEnd(renderDevice);

    rendering::IShaderConstantDatabase const* shaderConstantDatabase = iCompositing->GetCurrentShaderConstantDatabase();
    rendering::IShaderResourceDatabase const* shaderResourceDatabase = iCompositing->GetCurrentShaderResourceDatabase();

    rendering::ShaderResourceDatabase mipmapShaderResourceDatabase;
    mipmapShaderResourceDatabase.AddResource(m_descriptor->m_bindingPoint, m_surface.get());
    rendering::ShaderResourceDatabasePair<false, false> localShaderResourceDatabase(&mipmapShaderResourceDatabase, shaderResourceDatabase);

    ID3D11ShaderResourceView* ps_srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
    size_t ps_srvCount = SG_ARRAYSIZE(ps_srvs);
    size_t ps_srvMipmapIndex = all_ones;

    {
        ID3D11ShaderReflection* reflection = m_pixelShader.GetReflection();
        m_psConstantBuffers->UpdateIFN(renderDevice, reflection, shaderConstantDatabase);
        ID3D11Buffer** buffers = m_psConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_psConstantBuffers->BufferCount();
        context->PSSetConstantBuffers(
            0,
            bufferCount,
            buffers);

        m_psResources->UpdateIFN(reflection, &localShaderResourceDatabase);
        m_psResources->GetShaderResourceViews(ps_srvCount, ps_srvs);
        for(size_t i = 0; i < ps_srvCount; ++i)
            if(ps_srvs[i] == m_surface->GetShaderResourceView())
                ps_srvMipmapIndex = i;
        SG_ASSERT(-1 != ps_srvMipmapIndex);

        ID3D11SamplerState* sss[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        size_t ssCount = SG_ARRAYSIZE(sss);
        m_psResources->GetSamplers(ssCount, sss);
        context->PSSetSamplers( 0, checked_numcastable(ssCount), sss );
    }
    {
        ID3D11ShaderReflection* reflection = m_vertexShader.GetReflection();
        m_vsConstantBuffers->UpdateIFN(renderDevice, reflection, shaderConstantDatabase);
        ID3D11Buffer** buffers = m_vsConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_vsConstantBuffers->BufferCount();
        context->VSSetConstantBuffers(
            0,
            bufferCount,
            buffers);

        m_vsResources->UpdateIFN(reflection, shaderResourceDatabase);
        ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        size_t srvCount = SG_ARRAYSIZE(srvs);
        m_vsResources->GetShaderResourceViews(srvCount, srvs);
        context->VSSetShaderResources( 0, checked_numcastable(srvCount), srvs );

        ID3D11SamplerState* sss[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        size_t ssCount = SG_ARRAYSIZE(sss);
        m_vsResources->GetSamplers(ssCount, sss);
        context->VSSetSamplers( 0, checked_numcastable(ssCount), sss );
    }

    context->VSSetShader( m_vertexShader.GetShader(), NULL, 0 );
    context->PSSetShader( m_pixelShader.GetShader(), NULL, 0 );

    context->OMSetBlendState( m_blendState.get(), NULL, 0xFFFFFFFF );

    uint2 wh = m_surface->Resolution()->Get();

    size_t const mipCount = m_surface->MipCount();
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
            m_surface->GetMipLevelRenderTargetView(i),
        };
        size_t const rtvCount = SG_ARRAYSIZE(rtvs);
        renderDevice->ImmediateContext()->OMSetRenderTargets( rtvCount, rtvs, NULL );

        ps_srvs[ps_srvMipmapIndex] = m_surface->GetMipLevelShaderResourceView(i-1);
        context->PSSetShaderResources( 0, checked_numcastable(ps_srvCount), ps_srvs );

        iCompositing->GetQuadForShaderPass()->Execute(iCompositing);

        ID3D11ShaderResourceView* clearsrvs[] = {
            nullptr,
        };
        size_t const clearsrvCount = SG_ARRAYSIZE(clearsrvs);
        context->PSSetShaderResources( 0, clearsrvCount, clearsrvs );
    }
}
//=============================================================================
GenerateMipmapDescriptor::GenerateMipmapDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
GenerateMipmapDescriptor::~GenerateMipmapDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
GenerateMipmap* GenerateMipmapDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new GenerateMipmap(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void GenerateMipmapDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    REFLECTION_CHECK_PROPERTY_NotNull(surface);
    REFLECTION_CHECK_PROPERTY_NotNull(pixelShader);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), GenerateMipmapDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(surface, "")
    REFLECTION_m_PROPERTY_DOC(bindingPoint, "binding name for previous mip in pixel shader")
    REFLECTION_m_PROPERTY_DOC(pixelShader, "")
    REFLECTION_m_PROPERTY_DOC(vertexShader, "")
REFLECTION_CLASS_END
//=============================================================================
SetRenderTargets::SetRenderTargets(SetRenderTargetsDescriptor const* iDescriptor, Compositing* iCompositing)
    : m_descriptor(iDescriptor)
{
    m_renderTargets.reserve(m_descriptor->m_renderTargets.size());
    for(auto const& it : m_descriptor->m_renderTargets)
    {
        m_renderTargets.push_back(it->GetAsRenderTarget(iCompositing));
        SG_ASSERT(nullptr != m_renderTargets.back());
    }
    if(nullptr != m_descriptor->m_depthStencil)
        m_depthStencil = m_descriptor->m_depthStencil->GetAsSurface(iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetRenderTargets::~SetRenderTargets()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetRenderTargets::Execute(Compositing* iCompositing)
{
    size_t const MAX_RTV_COUNT = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
    ID3D11RenderTargetView* rtvs[MAX_RTV_COUNT];
    size_t const rtvCount = m_renderTargets.size();
    SG_ASSERT(rtvCount < MAX_RTV_COUNT);
    uint2 const resolution = rtvCount > 0 ? m_renderTargets[0]->RenderTargetResolution()->Get() : uint2(0);
    for(size_t i = 0; i < rtvCount; ++i)
    {
        rtvs[i] = m_renderTargets[i]->GetRenderTargetView();
        SG_ASSERT(m_renderTargets[i]->RenderTargetResolution()->Get() == resolution);
    }
    SG_ASSERT(nullptr == m_depthStencil || m_depthStencil->Resolution()->Get() == resolution);
    ID3D11DepthStencilView* dsv = nullptr == m_depthStencil ? NULL : m_depthStencil->GetDepthStencilView();

    rendering::RenderDevice const* renderDevice = iCompositing->RenderDevice();
    ID3D11DeviceContext* d3dContext = renderDevice->ImmediateContext();
    d3dContext->OMSetRenderTargets( (UINT)rtvCount, rtvs, dsv );

    if(rtvCount > 0)
    {
        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)resolution.x();
        vp.Height = (FLOAT)resolution.y();
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        d3dContext->RSSetViewports( 1, &vp );

        // Setup a scissor rect
        D3D11_RECT rect;
        rect.left = LONG(vp.TopLeftX);
        rect.top = LONG(vp.TopLeftY);
        rect.right = LONG(vp.Width - vp.TopLeftX);
        rect.bottom = LONG(vp.Height - vp.TopLeftY);
        d3dContext->RSSetScissorRects( 1, &rect );
    }
}
//=============================================================================
SetRenderTargetsDescriptor::SetRenderTargetsDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetRenderTargetsDescriptor::~SetRenderTargetsDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetRenderTargets* SetRenderTargetsDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new SetRenderTargets(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SetRenderTargetsDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(renderTargets, "")
    REFLECTION_m_PROPERTY_DOC(depthStencil, "")
REFLECTION_CLASS_END
//=============================================================================
ShaderPass::ShaderPass(ShaderPassDescriptor const* iDescriptor, Compositing* iCompositing)
    : m_descriptor(iDescriptor)
    , m_psConstantBuffers(new rendering::ShaderConstantBuffers())
    , m_vsConstantBuffers(new rendering::ShaderConstantBuffers())
    , m_psResources(new rendering::ShaderResourceBuffer())
    , m_vsResources(new rendering::ShaderResourceBuffer())
{
    m_pixelShader = m_descriptor->m_pixelShader->GetProxy();
    if(nullptr != m_descriptor->m_vertexShader)
        m_vertexShader = m_descriptor->m_vertexShader->GetProxy();
    else
        m_vertexShader = iCompositing->GetQuadForShaderPass()->DefaultVertexShader();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderPass::~ShaderPass()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void ShaderPass::Execute(Compositing* iCompositing)
{
    rendering::RenderDevice const* renderDevice = iCompositing->RenderDevice();
    ID3D11DeviceContext* context = renderDevice->ImmediateContext();
    SG_ASSERT(nullptr != context);

    rendering::IShaderConstantDatabase const* shaderConstantDatabase = iCompositing->GetCurrentShaderConstantDatabase();
    rendering::IShaderResourceDatabase const* shaderResourceDatabase = iCompositing->GetCurrentShaderResourceDatabase();

    {
        ID3D11ShaderReflection* reflection = m_pixelShader.GetReflection();
        m_psConstantBuffers->UpdateIFN(renderDevice, reflection, shaderConstantDatabase);
        ID3D11Buffer** buffers = m_psConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_psConstantBuffers->BufferCount();
        context->PSSetConstantBuffers(
            0,
            bufferCount,
            buffers);

        m_psResources->UpdateIFN(reflection, shaderResourceDatabase);
        ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        size_t srvCount = SG_ARRAYSIZE(srvs);
        m_psResources->GetShaderResourceViews(srvCount, srvs);
        context->PSSetShaderResources( 0, checked_numcastable(srvCount), srvs );

        ID3D11SamplerState* sss[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        size_t ssCount = SG_ARRAYSIZE(sss);
        m_psResources->GetSamplers(ssCount, sss);
        context->PSSetSamplers( 0, checked_numcastable(ssCount), sss );
    }
    {
        ID3D11ShaderReflection* reflection = m_vertexShader.GetReflection();
        m_vsConstantBuffers->UpdateIFN(renderDevice, reflection, shaderConstantDatabase);
        ID3D11Buffer** buffers = m_vsConstantBuffers->Buffers();
        UINT bufferCount = (UINT)m_vsConstantBuffers->BufferCount();
        context->VSSetConstantBuffers(
            0,
            bufferCount,
            buffers);

        m_vsResources->UpdateIFN(reflection, shaderResourceDatabase);
        ID3D11ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        size_t srvCount = SG_ARRAYSIZE(srvs);
        m_vsResources->GetShaderResourceViews(srvCount, srvs);
        context->VSSetShaderResources( 0, checked_numcastable(srvCount), srvs );

        ID3D11SamplerState* sss[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
        size_t ssCount = SG_ARRAYSIZE(sss);
        m_vsResources->GetSamplers(ssCount, sss);
        context->VSSetSamplers( 0, checked_numcastable(ssCount), sss );
    }

    context->PSSetShader( m_pixelShader.GetShader(), NULL, 0 );
    context->VSSetShader( m_vertexShader.GetShader(), NULL, 0 );

    iCompositing->GetQuadForShaderPass()->Execute(iCompositing);
}
//=============================================================================
ShaderPassDescriptor::ShaderPassDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderPassDescriptor::~ShaderPassDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderPass* ShaderPassDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new ShaderPass(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void ShaderPassDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    REFLECTION_CHECK_PROPERTY_NotNull(pixelShader);
    REFLECTION_CHECK_PROPERTY(pixelShader, m_pixelShader->IsValid());
    if(nullptr != m_vertexShader)
    {
        REFLECTION_CHECK_PROPERTY(vertexShader, m_vertexShader->IsValid());
    }
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), ShaderPassDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(pixelShader, "")
    REFLECTION_m_PROPERTY_DOC(vertexShader, "")
REFLECTION_CLASS_END
//=============================================================================
SetConstantDatabase::SetConstantDatabase(SetConstantDatabaseDescriptor const* iDescriptor, Compositing* iCompositing)
: m_descriptor(iDescriptor)
, m_database()
{
    m_database = iCompositing->GetShaderConstantDatabase(m_descriptor->m_database.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetConstantDatabase::~SetConstantDatabase()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetConstantDatabase::Execute(Compositing* iCompositing)
{
    iCompositing->SetCurrentShaderConstantDatabase(m_database.get());
}
//=============================================================================
SetConstantDatabaseDescriptor::SetConstantDatabaseDescriptor()
: m_database()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetConstantDatabaseDescriptor::~SetConstantDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetConstantDatabase* SetConstantDatabaseDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new SetConstantDatabase(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void SetConstantDatabaseDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    SG_ASSERT(nullptr != m_database);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SetConstantDatabaseDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(database, "")
REFLECTION_CLASS_END
//=============================================================================
SetShaderResourceDatabase::SetShaderResourceDatabase(SetShaderResourceDatabaseDescriptor const* iDescriptor, Compositing* iCompositing)
: m_descriptor(iDescriptor)
, m_database()
{
    m_database = iCompositing->GetShaderResourceDatabase(m_descriptor->m_database.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetShaderResourceDatabase::~SetShaderResourceDatabase()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetShaderResourceDatabase::Execute(Compositing* iCompositing)
{
    iCompositing->SetCurrentShaderResourceDatabase(m_database.get());
}
//=============================================================================
SetShaderResourceDatabaseDescriptor::SetShaderResourceDatabaseDescriptor()
: m_database()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetShaderResourceDatabaseDescriptor::~SetShaderResourceDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetShaderResourceDatabase* SetShaderResourceDatabaseDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new SetShaderResourceDatabase(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void SetShaderResourceDatabaseDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    SG_ASSERT(nullptr != m_database);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SetShaderResourceDatabaseDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(database, "")
REFLECTION_CLASS_END
//=============================================================================
}
}


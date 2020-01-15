#include "stdafx.h"

#include "CompositingInstruction.h"

#include "Compositing.h"
#include "DatabaseDescriptors.h"
#include "ShaderDescriptors.h"
#include "SurfaceDescriptors.h"
#include <Core/WinUtils.h>
#include <Image/ImageIteration.h>
#include <FileFormats/pnm.h>
#include <Reflection/CommonTypes.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/RenderStateUtils.h>
#include <Rendering/ShaderConstantBuffers.h>
#include <Rendering/ShaderConstantDatabase.h>
#include <Rendering/ShaderResourceDatabase.h>
#include <Rendering/Surface.h>
#include <Rendering/VertexTypes.h>
#include <sstream>

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
SetBlendState::SetBlendState(SetBlendStateDescriptor const* iDescriptor, Compositing* iCompositing)
    : m_descriptor(iDescriptor)
{
    SG_UNUSED(iCompositing);
    if(m_descriptor->m_preregisteredName.IsValid())
        m_blendState = rendering::renderstatedico::GetBlendState(m_descriptor->m_preregisteredName);
    SG_ASSERT(nullptr != m_blendState || !m_descriptor->m_preregisteredName.IsValid());
    if(nullptr == m_blendState)
        m_blendState = iCompositing->RenderDevice()->DefaultBlendState();
    SG_ASSERT(nullptr != m_blendState);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetBlendState::~SetBlendState()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetBlendState::Execute(Compositing* iCompositing)
{
    ID3D11DeviceContext* immediateContext = iCompositing->RenderDevice()->ImmediateContext();
    immediateContext->OMSetBlendState( m_blendState.get(), NULL, 0xFFFFFFFF );
}
//=============================================================================
SetBlendStateDescriptor::SetBlendStateDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetBlendStateDescriptor::~SetBlendStateDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetBlendState* SetBlendStateDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new SetBlendState(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SetBlendStateDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(preregisteredName, "")
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
SwapSurfaces::SwapSurfaces(SwapSurfacesDescriptor const* iDescriptor, Compositing* iCompositing)
    : m_descriptor(iDescriptor)
{
    m_surfaces.Reserve(m_descriptor->m_surfaces.Size());
    for(auto const& it : m_descriptor->m_surfaces)
    {
        m_surfaces.EmplaceBack_AssumeCapacity(iCompositing->GetMutableSurface(it.first.get()), iCompositing->GetMutableSurface(it.second.get()));
        SG_ASSERT(m_surfaces.Back().first->Resolution() == m_surfaces.Back().second->Resolution());
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SwapSurfaces::~SwapSurfaces()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SwapSurfaces::Execute(Compositing* iCompositing)
{
    SG_UNUSED(iCompositing);
    for(auto const& it : m_surfaces)
    {
        swap(*it.first, *it.second);
    }
}
//=============================================================================
SwapSurfacesDescriptor::SwapSurfacesDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SwapSurfacesDescriptor::~SwapSurfacesDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SwapSurfaces* SwapSurfacesDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new SwapSurfaces(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void SwapSurfacesDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
#if SG_ENABLE_ASSERT
    for(auto const& it : m_surfaces)
    {
        SG_ASSERT(it.first->ResolutionDescriptor() == it.second->ResolutionDescriptor());
    }
#endif
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SwapSurfacesDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(surfaces, "lst of pairs of surfaces")
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
    m_blendState = rendering::renderstatedico::GetBlendState(iCompositing->RenderDevice()->RenderStateName_NoBlend());
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
SetAliasesInConstantDatabase::SetAliasesInConstantDatabase(SetAliasesInConstantDatabaseDescriptor const* iDescriptor, Compositing* iCompositing)
    : m_descriptor(iDescriptor)
    , m_database()
{
    m_database = iCompositing->GetShaderConstantDatabase(m_descriptor->m_database.get());
    m_writableDatabase = iCompositing->GetWritableShaderConstantDatabase(m_descriptor->m_database.get());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetAliasesInConstantDatabase::~SetAliasesInConstantDatabase()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetAliasesInConstantDatabase::Execute(Compositing* iCompositing)
{
    SG_UNUSED(iCompositing);
    for(auto const& alias : m_descriptor->m_aliases)
    {
        if(alias.second.empty())
        {
            m_writableDatabase->RemoveVariable(alias.first);
        }
        else
        {
             rendering::IShaderVariable const* variable = m_database->GetConstantIFP(alias.second);
             SG_ASSERT_MSG(nullptr != variable, "variable not in database");
             m_writableDatabase->AddReference(alias.first, variable);
        }
    }
}
//=============================================================================
SetAliasesInConstantDatabaseDescriptor::SetAliasesInConstantDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetAliasesInConstantDatabaseDescriptor::~SetAliasesInConstantDatabaseDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SetAliasesInConstantDatabase* SetAliasesInConstantDatabaseDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new SetAliasesInConstantDatabase(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void SetAliasesInConstantDatabaseDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
    SG_ASSERT(nullptr != m_database);
    SG_ASSERT(!m_aliases.empty());
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), SetAliasesInConstantDatabaseDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(database, "")
    REFLECTION_m_PROPERTY_DOC(aliases, "")
REFLECTION_CLASS_END
//=============================================================================
DebugDumpSurfaces::DebugDumpSurfaces(DebugDumpSurfacesDescriptor const* iDescriptor, Compositing* iCompositing)
    : m_descriptor(iDescriptor)
{
    m_surfaceReaders.reserve(m_descriptor->m_surfaces.size());
    for(auto const& it : m_descriptor->m_surfaces)
    {
        rendering::Surface const* surface = iCompositing->GetSurface(it.second.get());
        m_surfaceReaders.EmplaceBack(new rendering::CPUSurfaceReader(iCompositing->RenderDevice(), surface));
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
DebugDumpSurfaces::~DebugDumpSurfaces()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
template <typename T, int N>
void DebugDumpSurfaceImpl(FilePath const& directory, std::string const& baseName, int index, rendering::CPUSurfaceReader* reader)
{
    typedef math::Vector<T,N> pixel_type;
    image::ImageView<pixel_type const> img;
    reader->GetImage(img);
    image::Image<ubyte3> rgbimg;
    image::Image<u8> alphaimg;
    if(SG_CONSTANT_CONDITION(N > 1))
        rgbimg.Reset(img.WidthHeight());
    if(SG_CONSTANT_CONDITION(N == 1 || N == 4))
        alphaimg.Reset(img.WidthHeight());
    if(SG_CONSTANT_CONDITION(std::is_same<T, u8>::value))
    {
        if(SG_CONSTANT_CONDITION(N == 1))
            image::CoIterate([&](u8& o, pixel_type const& i) { o = u8(i[0]); }, alphaimg.View(), img);
        int const Nrgb = std::max(3, N);
        if(SG_CONSTANT_CONDITION(N > 1))
            image::CoIterate([&](ubyte3& o, pixel_type const& i) { for_range(size_t, k, 0, Nrgb) { o[k] = u8(i[k]); } for_range(size_t, k, Nrgb, N) { o[k] = 0; } }, rgbimg.View(), img);
        if(SG_CONSTANT_CONDITION(N == 4))
            image::CoIterate([&](u8& o, pixel_type const& i) { o = u8(i[3]); }, alphaimg.View(), img);
    }
    else
    {
        pixel_type componentMin(std::numeric_limits<T>::max());
        pixel_type componentMax(std::numeric_limits<T>::min());
        image::CoIterate([&](pixel_type const& p) { componentMin = componentwise::min(componentMin, p); componentMax = componentwise::max(componentMax, p); }, img);
        T allMin = componentMin[0]; for_range(size_t, i, 1, N) allMin = std::min(allMin, componentMin[i]);
        T allMax = componentMax[0]; for_range(size_t, i, 1, N) allMax = std::max(allMax, componentMax[i]);
        pixel_type rangeMin = componentMin;
        pixel_type rangeMax = componentMax;
        if(allMin >= 0 && allMax <= 1)
        {
            rangeMin = pixel_type(0);
            rangeMax = pixel_type(1);
        }
        rangeMax = componentwise::max(rangeMin + 1, rangeMax);
        math::Vector<float, N> const ooRange = 255.f / (rangeMax-rangeMin);
        if(SG_CONSTANT_CONDITION(N == 1))
            image::CoIterate([&](u8& o, pixel_type const& i) { o = u8(clamp(roundi((i[0] - rangeMin[0]) * ooRange[0]), 0, 255)); }, alphaimg.View(), img);
        int const Nrgb = std::min(3, N);
        if(SG_CONSTANT_CONDITION(N > 1))
            image::CoIterate([&](ubyte3& o, pixel_type const& i) { for_range(size_t, k, 0, Nrgb) { o[k] = u8(clamp(roundi((i[k] - rangeMin[k]) * ooRange[k]), 0, 255)); } for_range(size_t, k, Nrgb, N) { o[k] = 0; } }, rgbimg.View(), img);
        if(SG_CONSTANT_CONDITION(N == 4))
            image::CoIterate([&](u8& o, pixel_type const& i) { o = u8(clamp(roundi((i[3] - rangeMin[3]) * ooRange[3]), 0, 255)); }, alphaimg.View(), img);

        {
            FilePath path = directory.Append(Format("%0_%1.txt", baseName, index));
            std::ostringstream oss;
            oss << "size: " << img.WidthHeight().x() << " x " << img.WidthHeight().y() << std::endl;
            oss << "channel count: " << N << std::endl;
            oss << "min: "; for_range(size_t, k, 0, N) { oss << " " << componentMin[k]; } oss << std::endl;
            oss << "max: "; for_range(size_t, k, 0, N) { oss << " " << componentMax[k]; } oss << std::endl;
            if(SG_CONSTANT_CONDITION(!std::is_same<T, u8>::value || !std::is_same<T, u8>::value))
            {
                for_range(size_t, j, 0, img.WidthHeight().y())
                {
                    for_range(size_t, i, 0, img.WidthHeight().x())
                    {
                        for_range(size_t, k, 0, N)
                        {
                            oss << " ";
                            oss << img(i,j)[k];
                        }
                        oss << ",";
                    }
                    oss << std::endl;
                }
            }
#if SG_PLATFORM_IS_WIN
            winutils::CreateDirectoryPathIFN(path.ParentDirectory().GetSystemFilePath());
            bool ok = winutils::WriteFileOverwriteIFNROK(path.GetSystemFilePath(), reinterpret_cast<u8 const*>(oss.str().data()), oss.str().size());
            SG_ASSERT_AND_UNUSED(ok);
#else
#error todo
            SG_ASSERT_NOT_IMPLEMENTED();
#endif
        }
    }

    if(SG_CONSTANT_CONDITION(N == 1))
    {
        FilePath path = directory.Append(Format("%0_%1.pnm", baseName, index));
        pnm::WriteGrayROK(path, alphaimg.Buffer(), alphaimg.BufferSizeInBytes(), alphaimg.WidthHeight().x(), alphaimg.WidthHeight().y(), alphaimg.StrideInBytes());
    }
    if(SG_CONSTANT_CONDITION(N > 1 && N < 4))
    {
        FilePath path = directory.Append(Format("%0_%1.pnm", baseName, index));
        pnm::WriteRGBROK(path, rgbimg.Buffer(), rgbimg.BufferSizeInBytes(), rgbimg.WidthHeight().x(), rgbimg.WidthHeight().y(), rgbimg.StrideInBytes());
    }
    if(SG_CONSTANT_CONDITION(N == 4))
    {
        FilePath pathrgb = directory.Append(Format("%0_%1_rgb.pnm", baseName, index));
        pnm::WriteRGBROK(pathrgb, rgbimg.Buffer(), rgbimg.BufferSizeInBytes(), rgbimg.WidthHeight().x(), rgbimg.WidthHeight().y(), rgbimg.StrideInBytes());
        FilePath pathalpha = directory.Append(Format("%0_%1_alpha.pnm", baseName, index));
        pnm::WriteGrayROK(pathalpha, alphaimg.Buffer(), alphaimg.BufferSizeInBytes(), alphaimg.WidthHeight().x(), alphaimg.WidthHeight().y(), alphaimg.StrideInBytes());
    }
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void DebugDumpSurfaces::Execute(Compositing* iCompositing)
{
    if(m_count > m_descriptor->m_stop)
        return;
    bool const doDump = m_count <= m_descriptor->m_stop ? (m_iter - m_descriptor->m_start) % (m_descriptor->m_skip + 1) == 0 : false;

    if(doDump)
    {
        for_range(size_t, i, 0, m_descriptor->m_surfaces.size())
        {
            rendering::CPUSurfaceReader* surfaceReader = m_surfaceReaders[i].get();
            surfaceReader->Update();
            surfaceReader->Map();
            std::string const& basename = m_descriptor->m_surfaces[i].first;
            rendering::Surface const* surface = iCompositing->GetSurface(m_descriptor->m_surfaces[i].second.get());
            rendering::SurfaceFormat const surfaceFormat = surface->GetSurfaceFormat();
            switch(surfaceFormat)
            {
            case rendering::SurfaceFormat::R32G32B32A32_TYPELESS : DebugDumpSurfaceImpl<float, 4>(m_descriptor->m_directory, basename, m_iter, surfaceReader); break;
            case rendering::SurfaceFormat::R32G32_TYPELESS :       DebugDumpSurfaceImpl<float, 2>(m_descriptor->m_directory, basename, m_iter, surfaceReader); break;
            default:
                SG_ASSERT_NOT_IMPLEMENTED();
            }
            surfaceReader->Unmap();
        }

        ++m_count;
    }
    ++m_iter;
}
//=============================================================================
DebugDumpSurfacesDescriptor::DebugDumpSurfacesDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
DebugDumpSurfacesDescriptor::~DebugDumpSurfacesDescriptor()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
DebugDumpSurfaces* DebugDumpSurfacesDescriptor::CreateInstance(Compositing* iCompositing) const
{
    return new DebugDumpSurfaces(this, iCompositing);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if ENABLE_REFLECTION_PROPERTY_CHECK
void DebugDumpSurfacesDescriptor::VirtualCheckProperties(reflection::ObjectPropertyCheckContext& iContext) const
{
    reflection_parent_type::VirtualCheckProperties(iContext);
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
REFLECTION_CLASS_BEGIN((sg, renderengine), DebugDumpSurfacesDescriptor)
REFLECTION_CLASS_DOC("")
    REFLECTION_m_PROPERTY_DOC(directory, "")
    REFLECTION_m_PROPERTY_DOC(surfaces, "")
    REFLECTION_m_PROPERTY_DOC(start, "")
    REFLECTION_m_PROPERTY_DOC(skip, "")
    REFLECTION_m_PROPERTY_DOC(stop, "")
REFLECTION_CLASS_END
//=============================================================================
}
}


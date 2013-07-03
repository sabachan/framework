#include "stdafx.h"

#include "Surface.h"

#include <algorithm>
#if SG_PLATFORM_IS_WIN
#include <Core/WindowsH.h>
#include <d3d11.h>
#include <dxgi.h>
#endif
#include "RenderDevice.h"

namespace sg {
namespace rendering {
//=============================================================================
BaseSurface::BaseSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const& iProperties, bool iIsBC, bool iIsDepthStencil)
    : m_renderDevice(iRenderDevice)
    , m_resolutionServer(iResolutionServer)
    , m_texture2D()
    , m_shaderResourceView()
    , m_mipShaderResourceViews()
    , m_mipRenderTargetViews()
    , m_properties(iProperties)
    , m_isBC(iIsBC)
    , m_isDepthStencil(iIsDepthStencil)
    SG_CODE_FOR_ASSERT(SG_COMMA m_generatedMip(all_ones))
{
    SG_ASSERT(!m_isDepthStencil || !m_isBC);
    SG_ASSERT(DXGI_FORMAT_UNKNOWN != m_properties.readFormat || m_isDepthStencil);
    SG_ASSERT(DXGI_FORMAT_UNKNOWN != m_properties.writeFormat);
    m_resolutionServer->RegisterObserver(this);
    CreateObjectsIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
BaseSurface::~BaseSurface()
{
    if(nullptr != m_resolutionServer)
        Terminate();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseSurface::Terminate()
{
    SG_ASSERT(nullptr != m_resolutionServer);
    ReleaseObjectsIFN();
    m_resolutionServer->UnregisterObserver(this);
    m_resolutionServer = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseSurface::VirtualOnNotified(ResolutionServer const* iResolutionServer)
{
    SG_ASSERT_AND_UNUSED(iResolutionServer == m_resolutionServer);

    ReleaseObjectsIFN();
    CreateObjectsIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseSurface::ReleaseObjectsIFN()
{
    m_mipRenderTargetViews.clear();
    m_mipShaderResourceViews.clear();
    m_shaderResourceView = nullptr;
    m_texture2D = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
u32 Log2(u32 n)
{
    // There is an intrisic to speed up that, but I will search for it after and this code is portable.
    u32 l = 0;
    u32 t;
    t = n >> 16; if(t) { l += 16; n = t; }
    t = n >>  8; if(t) { l +=  8; n = t; }
    t = n >>  4; if(t) { l +=  4; n = t; }
    t = n >>  2; if(t) { l +=  2; n = t; }
    t = n >>  1; if(t) { l +=  1; n = t; }
    return l;
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void BaseSurface::CreateObjectsIFN()
{
    SG_ASSERT(IsValid_ForAssert());
    SG_ASSERT(nullptr == m_texture2D);
    uint2 const wh = m_resolutionServer->Get();

    if(wh.x() > 0 && wh.y() > 0)
    {
        HRESULT hr;

        ID3D11Device* device = m_renderDevice->D3DDevice();
        SG_ASSERT(nullptr != device);

        uint2 basewh = wh;
        if(m_isBC)
        {
            SG_ASSERT((wh.x() & 3) == 0 && (wh.y() & 3) == 0 );
            basewh = wh >> uint2(2);
            m_blocResolutionServer.Set(basewh);
        }
        else
            SG_ASSERT(m_blocResolutionServer.Get() == uint2(0));

        D3D11_TEXTURE2D_DESC textureDesc;
        textureDesc.Width = basewh.x();
        textureDesc.Height = basewh.y();
        SG_ASSERT(1 == m_properties.mipLevels || !m_isDepthStencil);
        textureDesc.MipLevels = (UINT)m_properties.mipLevels;
        textureDesc.ArraySize = 1;
        textureDesc.Format = m_properties.baseFormat;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT; // TODO : expose ? (but BindFlags and CPUAccessFlags must be also changed)
        textureDesc.BindFlags = 0;
        if(DXGI_FORMAT_UNKNOWN != m_properties.readFormat)
            textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        SG_ASSERT(DXGI_FORMAT_UNKNOWN != m_properties.writeFormat);
        if(m_isDepthStencil)
            textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
        else
            textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
        hr = device->CreateTexture2D(&textureDesc, NULL, m_texture2D.GetPointerForInitialisation());
        SG_ASSERT(SUCCEEDED(hr));

#if SG_ENABLE_ASSERT
        rendering::SetDebugName(m_texture2D.get(), "Surface");
#endif

        if(DXGI_FORMAT_UNKNOWN != m_properties.readFormat)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format = m_properties.readFormat;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = UINT(-1);

            hr = device->CreateShaderResourceView(
                m_texture2D.get(),
                &srvDesc,
                m_shaderResourceView.GetPointerForInitialisation()
                );
            SG_ASSERT(SUCCEEDED(hr));
        }

        if(m_isDepthStencil)
        {

            SG_ASSERT(DXGI_FORMAT_UNKNOWN != m_properties.writeFormat);
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format = m_properties.writeFormat;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Flags = 0; // Note: read only depth buffers allow binding of multiple depth-buffer. Must this be supported ?
            dsvDesc.Texture2D.MipSlice = 0;

            hr = device->CreateDepthStencilView(
                m_texture2D.get(),
                &dsvDesc,
                m_depthStencilView.GetPointerForInitialisation()
                );
            SG_ASSERT(SUCCEEDED(hr));
        }
        else
        {
            SG_ASSERT(DXGI_FORMAT_UNKNOWN != m_properties.writeFormat);
            size_t const maxMipLevels = Log2(std::max(basewh.x(), basewh.y())); // To check
            size_t const totalMipLevels = 0==textureDesc.MipLevels ? maxMipLevels : std::min((size_t)m_properties.mipLevels, maxMipLevels);
            SG_ASSERT(m_mipShaderResourceViews.empty());
            SG_ASSERT(m_mipRenderTargetViews.empty());
            m_mipShaderResourceViews.resize(totalMipLevels);
            m_mipRenderTargetViews.resize(totalMipLevels);
            for(size_t i = 0; i < totalMipLevels; ++i)
            {
                SG_ASSERT(DXGI_FORMAT_UNKNOWN != m_properties.readFormat);
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
                srvDesc.Format = m_properties.readFormat;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = (UINT)i;
                srvDesc.Texture2D.MipLevels = 1;

                hr = device->CreateShaderResourceView(
                    m_texture2D.get(),
                    &srvDesc,
                    m_mipShaderResourceViews[i].GetPointerForInitialisation()
                    );
                SG_ASSERT(SUCCEEDED(hr));

                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
                rtvDesc.Format = m_properties.writeFormat;
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = (UINT)i;

                hr = device->CreateRenderTargetView(
                    m_texture2D.get(),
                    &rtvDesc,
                    m_mipRenderTargetViews[i].GetPointerForInitialisation());
                SG_ASSERT(SUCCEEDED(hr));
            }
        }
        SG_CODE_FOR_ASSERT(m_generatedMip = -1);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11ShaderResourceView* BaseSurface::GetMipLevelShaderResourceView(size_t iMipLevel) const
{
    SG_ASSERT(IsValid_ForAssert());
    SG_ASSERT((int)iMipLevel <= m_generatedMip);
    return m_mipShaderResourceViews[iMipLevel].get();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11RenderTargetView* BaseSurface::GetMipLevelRenderTargetView(size_t iMipLevel) const
{
    SG_ASSERT(IsValid_ForAssert());
    SG_CODE_FOR_ASSERT(m_generatedMip = std::max((int)iMipLevel SG_COMMA m_generatedMip);)
    return m_mipRenderTargetViews[iMipLevel].get();
}
//=============================================================================
namespace {
    SurfaceProperties defaultSurfaceProperties(
        1,
        DXGI_FORMAT_B8G8R8A8_TYPELESS,
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
    );
    SurfaceProperties defaultBCSurfaceProperties(
        1,
        DXGI_FORMAT_R32G32_TYPELESS,
        DXGI_FORMAT_BC1_UNORM_SRGB,
        DXGI_FORMAT_R32G32_UINT
    );
    SurfaceProperties defaultDepthStencilSurfaceProperties(
        1,
        DXGI_FORMAT_R16_TYPELESS,
        DXGI_FORMAT_R16_UNORM,
        DXGI_FORMAT_D16_UNORM
    );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Surface::Surface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties)
: BaseSurface(iRenderDevice, iResolutionServer, nullptr == iOptionnalProperties ? defaultSurfaceProperties : *iOptionnalProperties, false, false)
{
    if(nullptr != iOptionnalProperties)
    {
        SG_ASSERT(DXGI_FORMAT_UNKNOWN != iOptionnalProperties->readFormat);
        SG_ASSERT(DXGI_FORMAT_UNKNOWN != iOptionnalProperties->writeFormat);
    }
}
//=============================================================================
BCSurface::BCSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties)
: BaseSurface(iRenderDevice, iResolutionServer, nullptr == iOptionnalProperties ? defaultBCSurfaceProperties : *iOptionnalProperties, true, false)
{
    if(nullptr != iOptionnalProperties)
    {
        SG_ASSERT(DXGI_FORMAT_UNKNOWN != iOptionnalProperties->readFormat);
        SG_ASSERT(DXGI_FORMAT_UNKNOWN != iOptionnalProperties->writeFormat);
    }
}
//=============================================================================
DepthStencilSurface::DepthStencilSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties)
: BaseSurface(iRenderDevice, iResolutionServer, nullptr == iOptionnalProperties ? defaultDepthStencilSurfaceProperties : *iOptionnalProperties, false, true)
{
    if(nullptr != iOptionnalProperties)
    {
        SG_ASSERT(1 == iOptionnalProperties->mipLevels);
        SG_ASSERT(DXGI_FORMAT_UNKNOWN != iOptionnalProperties->writeFormat);
    }
}
//=============================================================================
SurfaceView::SurfaceView(RenderDevice const* iRenderDevice, BaseSurface const* iBaseSurface, SurfaceProperties const& iProperties)
    : m_renderDevice(iRenderDevice)
    , m_baseSurface(iBaseSurface)
    , m_shaderResourceView()
    , m_renderTargetView()
    , m_properties(iProperties)
{
    SG_ASSERT_MSG(!m_baseSurface->m_isDepthStencil, "You shouldn't need another view for a depth stencil surface, do you?");
    SG_ASSERT_MSG(!m_baseSurface->m_isBC || DXGI_FORMAT_UNKNOWN != m_properties.writeFormat, "please use directly the BCSurface to write a block compressed image");
    m_baseSurface->RegisterObserver(this);
    CreateObjectsIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SurfaceView::~SurfaceView()
{
    if(nullptr != m_baseSurface)
        Terminate();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SurfaceView::Terminate()
{
    SG_ASSERT(nullptr != m_baseSurface);
    ReleaseObjectsIFN();
    m_baseSurface->UnregisterObserver(this);
    m_baseSurface = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SurfaceView::VirtualOnNotified(BaseSurface const* iBaseSurface)
{
    SG_ASSERT_AND_UNUSED(iBaseSurface == m_baseSurface);

    ReleaseObjectsIFN();
    CreateObjectsIFN();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SurfaceView::ReleaseObjectsIFN()
{
    m_shaderResourceView = nullptr;
    m_renderTargetView = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SurfaceView::CreateObjectsIFN()
{
    SG_ASSERT(IsValid_ForAssert());
    SG_ASSERT(nullptr == m_shaderResourceView);
    ID3D11Texture2D* texture2D = m_baseSurface->m_texture2D.get();

    if(nullptr != texture2D)
    {
        HRESULT hr;

        ID3D11Device* device = m_renderDevice->D3DDevice();
        SG_ASSERT(nullptr != device);


        if(DXGI_FORMAT_UNKNOWN != m_properties.readFormat)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format = m_properties.readFormat;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = UINT(-1);

            hr = device->CreateShaderResourceView(
                texture2D,
                &srvDesc,
                m_shaderResourceView.GetPointerForInitialisation()
                );
            SG_ASSERT(SUCCEEDED(hr));
        }

        if(DXGI_FORMAT_UNKNOWN != m_properties.writeFormat)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = m_properties.writeFormat;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = (UINT)0;

            hr = device->CreateRenderTargetView(
                texture2D,
                &rtvDesc,
                m_renderTargetView.GetPointerForInitialisation());
            SG_ASSERT(SUCCEEDED(hr));
        }
    }
}
//=============================================================================
}
}

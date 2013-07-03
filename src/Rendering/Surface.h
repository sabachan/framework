#ifndef Rendering_Surface_H
#define Rendering_Surface_H

#include <Core/ComPtr.h>
#include <Core/Observer.h>
#include "RenderTarget.h"
#include "ResolutionServer.h"
#include "ShaderResource.h"
#include <dxgiformat.h>

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

namespace sg {
namespace rendering {
//=============================================================================
class RenderDevice;
//=============================================================================
struct SurfaceProperties
{
    size_t mipLevels;
    DXGI_FORMAT baseFormat;
    DXGI_FORMAT readFormat;
    DXGI_FORMAT writeFormat;

    SurfaceProperties()
        : mipLevels(0)
        , baseFormat(DXGI_FORMAT_UNKNOWN)
        , readFormat(DXGI_FORMAT_UNKNOWN)
        , writeFormat(DXGI_FORMAT_UNKNOWN)
    {}
    SurfaceProperties(
        size_t iMipLevels,
        DXGI_FORMAT iBaseFormat,
        DXGI_FORMAT iReadFormat,
        DXGI_FORMAT iWriteFormat)
        : mipLevels(iMipLevels)
        , baseFormat(iBaseFormat)
        , readFormat(iReadFormat)
        , writeFormat(iWriteFormat)
    {}
};
//=============================================================================
class BaseSurface : public RefAndSafeCountable
                  , public Observable<BaseSurface>
                  , private Observer<ResolutionServer>
{
    PARENT_SAFE_COUNTABLE(RefAndSafeCountable)
    SG_NON_COPYABLE(BaseSurface)
    friend class SurfaceView;
public:
    virtual ~BaseSurface() override;
    void Terminate();
#if SG_ENABLE_ASSERT
    bool IsValid_ForAssert() const { return nullptr != m_resolutionServer; }
#endif
protected:
    BaseSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const& iProperties, bool iIsBC, bool iIsDepthStencil);
    ID3D11ShaderResourceView* ShaderResourceView() const { SG_ASSERT(IsValid_ForAssert()); return m_shaderResourceView.get(); }
    ID3D11RenderTargetView* RenderTargetView() const { SG_ASSERT(IsValid_ForAssert()); return GetMipLevelRenderTargetView(0); }
    ID3D11DepthStencilView* GetDepthStencilView() const { SG_ASSERT(IsValid_ForAssert()); return m_depthStencilView.get(); }
    ResolutionServer const* Resolution() const { SG_ASSERT(IsValid_ForAssert()); return m_resolutionServer.get(); }
    ResolutionServer const* BlocResolution() const { SG_ASSERT(IsValid_ForAssert()); return &m_blocResolutionServer; }
    size_t MipCount() const { SG_ASSERT(IsValid_ForAssert()); return m_mipShaderResourceViews.size(); }
    ID3D11ShaderResourceView* GetMipLevelShaderResourceView(size_t iMipLevel) const;
    ID3D11RenderTargetView* GetMipLevelRenderTargetView(size_t iMipLevel) const;
private:
    virtual void VirtualOnNotified(ResolutionServer const* iResolutionServer) override;
    void ReleaseObjectsIFN();
    void CreateObjectsIFN();
private:
    safeptr<RenderDevice const> const m_renderDevice;
    safeptr<ResolutionServer const> m_resolutionServer;
    ResolutionServer m_blocResolutionServer;
    SurfaceProperties const m_properties;
    comptr<ID3D11Texture2D> m_texture2D;
    comptr<ID3D11ShaderResourceView> m_shaderResourceView;
    comptr<ID3D11DepthStencilView> m_depthStencilView;
    std::vector<comptr<ID3D11ShaderResourceView> > m_mipShaderResourceViews;
    std::vector<comptr<ID3D11RenderTargetView> > m_mipRenderTargetViews;
    bool m_isBC;
    bool m_isDepthStencil;
    SG_CODE_FOR_ASSERT(mutable int m_generatedMip;)
};
//=============================================================================
class Surface : public BaseSurface
              , public IRenderTarget
              , public IShaderResource
{
    PARENT_REF_COUNTABLE(BaseSurface)
    PARENT_SAFE_COUNTABLE(BaseSurface)
public:
    Surface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties = nullptr);
    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { return ShaderResourceView(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return Resolution(); }
    virtual ID3D11RenderTargetView* GetRenderTargetView() const override { return RenderTargetView(); }
    virtual ResolutionServer const* RenderTargetResolution() const override { return Resolution(); }
    using BaseSurface::Resolution;
    using BaseSurface::MipCount;
    using BaseSurface::GetMipLevelShaderResourceView;
    using BaseSurface::GetMipLevelRenderTargetView;
};
//=============================================================================
class BCSurface : public BaseSurface
                , public IRenderTarget
                , public IShaderResource
{
    PARENT_REF_COUNTABLE(BaseSurface)
    PARENT_SAFE_COUNTABLE(BaseSurface)
public:
    BCSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties = nullptr);
    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { return ShaderResourceView(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return Resolution(); }
    virtual ID3D11RenderTargetView* GetRenderTargetView() const override { return RenderTargetView(); }
    virtual ResolutionServer const* RenderTargetResolution() const override { return BlocResolution(); }
    using BaseSurface::MipCount;
    using BaseSurface::GetMipLevelShaderResourceView;
    using BaseSurface::GetMipLevelRenderTargetView;
};
//=============================================================================
class DepthStencilSurface : public BaseSurface
                          , public IShaderResource
{
    PARENT_REF_COUNTABLE(BaseSurface)
    PARENT_SAFE_COUNTABLE(BaseSurface)
public:
    DepthStencilSurface(RenderDevice const* iRenderDevice, ResolutionServer const* iResolutionServer, SurfaceProperties const* iOptionnalProperties = nullptr);
    virtual ID3D11ShaderResourceView* GetShaderResourceView() const override { return ShaderResourceView(); }
    virtual ResolutionServer const* ShaderResourceResolution() const override { return Resolution(); }
    using BaseSurface::GetDepthStencilView;
    using BaseSurface::Resolution;
};
//=============================================================================
class SurfaceView : public RefAndSafeCountable
                  , private Observer<BaseSurface>
{
    PARENT_SAFE_COUNTABLE(RefAndSafeCountable)
    SG_NON_COPYABLE(SurfaceView
        )
public:
    virtual ~SurfaceView() override;
#if SG_ENABLE_ASSERT
    bool IsValid_ForAssert() const { return nullptr != m_baseSurface; }
#endif
protected:
    SurfaceView(RenderDevice const* iRenderDevice, BaseSurface const* iBaseSurface, SurfaceProperties const& iProperties);
    ID3D11ShaderResourceView* ShaderResourceView() const { SG_ASSERT(IsValid_ForAssert()); return m_shaderResourceView.get(); }
    ID3D11RenderTargetView* RenderTargetView() const { SG_ASSERT(IsValid_ForAssert()); return m_renderTargetView.get(); }
    ResolutionServer const* Resolution() const { SG_ASSERT(IsValid_ForAssert()); return m_baseSurface->Resolution(); }
private:
    void Terminate();
    virtual void VirtualOnNotified(BaseSurface const* iBaseSurface) override;
    void ReleaseObjectsIFN();
    void CreateObjectsIFN();
private:
    safeptr<RenderDevice const> const m_renderDevice;
    safeptr<BaseSurface const> m_baseSurface;
    SurfaceProperties const m_properties;
    comptr<ID3D11ShaderResourceView> m_shaderResourceView;
    comptr<ID3D11RenderTargetView> m_renderTargetView;
};
//=============================================================================
}
}

#endif
